//! Canonical SPHIST/1 history envelope (see experiments/flat/HISTORY-WIRE-V1.md).

use crate::{Event, EventKind, History, ObjectId, RuleId};
use std::collections::HashSet;

const MAGIC: &[u8; 8] = b"SPHIST1\0";

fn put_u32(out: &mut Vec<u8>, value: usize) -> Result<(), String> {
    let value = u32::try_from(value).map_err(|_| "SPHIST/1 collection too large".to_string())?;
    out.extend_from_slice(&value.to_be_bytes());
    Ok(())
}

fn put_string(out: &mut Vec<u8>, value: &str) -> Result<(), String> {
    put_u32(out, value.as_bytes().len())?;
    out.extend_from_slice(value.as_bytes());
    Ok(())
}

pub fn encode_history(
    initial: impl IntoIterator<Item = ObjectId>,
    rules: impl IntoIterator<Item = RuleId>,
    history: &History,
) -> Result<Vec<u8>, String> {
    let mut omega: Vec<_> = initial.into_iter().collect();
    omega.sort_unstable();
    if omega.windows(2).any(|pair| pair[0] == pair[1]) { return Err("duplicate initial option".into()); }
    let mut certified: Vec<_> = rules.into_iter().collect();
    certified.sort_unstable();
    if certified.windows(2).any(|pair| pair[0] == pair[1]) { return Err("duplicate certified rule".into()); }

    let mut out = MAGIC.to_vec();
    put_u32(&mut out, omega.len())?;
    for id in omega { out.extend_from_slice(&id.to_be_bytes()); }
    put_u32(&mut out, certified.len())?;
    for rule in certified { put_string(&mut out, rule)?; }
    put_u32(&mut out, history.len())?;
    for event in history.as_slice() {
        out.push(event.kind as u8);
        match event.kind {
            EventKind::Pop => out.extend_from_slice(&event.a.ok_or("Pop missing a")?.to_be_bytes()),
            EventKind::Refuse => {
                out.extend_from_slice(&event.a.ok_or("Refuse missing a")?.to_be_bytes());
                match event.b { Some(b) => { out.push(1); out.extend_from_slice(&b.to_be_bytes()); }, None => out.push(0) }
                put_string(&mut out, event.reason.as_deref().ok_or("Refuse missing reason")?)?;
            }
            EventKind::Bind => {
                out.extend_from_slice(&event.a.ok_or("Bind missing a")?.to_be_bytes());
                out.extend_from_slice(&event.b.ok_or("Bind missing b")?.to_be_bytes());
                put_string(&mut out, event.tag.as_deref().ok_or("Bind missing tag")?)?;
            }
            EventKind::Collapse => put_string(&mut out, event.rule.ok_or("Collapse missing rule")?)?,
        }
    }
    Ok(out)
}

struct Reader<'a> { data: &'a [u8], offset: usize }
impl<'a> Reader<'a> {
    fn take(&mut self, n: usize) -> Result<&'a [u8], String> { let end=self.offset.checked_add(n).ok_or("truncated SPHIST/1 envelope")?; if end>self.data.len(){return Err("truncated SPHIST/1 envelope".into())}; let value=&self.data[self.offset..end]; self.offset=end; Ok(value) }
    fn u8(&mut self) -> Result<u8,String> { Ok(self.take(1)?[0]) }
    fn u32(&mut self) -> Result<u32,String> { Ok(u32::from_be_bytes(self.take(4)?.try_into().unwrap())) }
    fn u64(&mut self) -> Result<u64,String> { Ok(u64::from_be_bytes(self.take(8)?.try_into().unwrap())) }
    fn string(&mut self) -> Result<&'static str,String> { let n=self.u32()? as usize; let value=std::str::from_utf8(self.take(n)?).map_err(|_|"invalid UTF-8 in SPHIST/1 envelope")?; Ok(Box::leak(value.to_string().into_boxed_str())) }
}

pub struct WireWorld { pub initial_option_space: HashSet<ObjectId>, pub certified_rules: Vec<RuleId>, pub history: History }

pub fn decode_history(data: &[u8]) -> Result<WireWorld, String> {
    let mut r=Reader{data,offset:0};
    if r.take(MAGIC.len())? != MAGIC { return Err("invalid SPHIST/1 magic".into()); }
    let mut omega=Vec::new(); for _ in 0..r.u32()? { omega.push(r.u64()?); }
    if omega.windows(2).any(|p|p[0]>=p[1]) { return Err("non-canonical option ordering".into()); }
    let mut rules=Vec::new(); for _ in 0..r.u32()? { rules.push(r.string()?); }
    if rules.windows(2).any(|p|p[0]>=p[1]) { return Err("non-canonical rule ordering".into()); }
    let mut history=History::new();
    for pos in 0..r.u32()? {
        let mut event=match r.u8()? {
            0 => Event::pop(r.u64()?),
            1 => { let a=r.u64()?; let flag=r.u8()?; let b=match flag {0=>None,1=>Some(r.u64()?),_=>return Err("invalid Refuse has_b flag".into())}; let reason=r.string()?.to_string(); match b {Some(b)=>Event::refuse_bind(a,b,reason),None=>Event::refuse(a,reason)} },
            2 => { let a=r.u64()?; let b=r.u64()?; Event::bind(a,b,r.string()?.to_string()) },
            3 => Event::collapse(r.string()?),
            kind => return Err(format!("invalid event kind {kind}")),
        };
        event.pos=pos as u64;
        history.push(event);
    }
    if r.offset != data.len() { return Err("trailing bytes in SPHIST/1 envelope".into()); }
    Ok(WireWorld{initial_option_space:omega.into_iter().collect(),certified_rules:rules,history})
}

pub fn fnv1a64(data: &[u8]) -> String {
    let mut digest=0xcbf29ce484222325u64;
    for byte in data { digest ^= *byte as u64; digest= digest.wrapping_mul(0x100000001b3); }
    format!("{digest:016x}")
}
