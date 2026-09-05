//! A minimal, dependency-free JSON reader/writer used only by the flat
//! fixture runner (`bin/fixtures.rs`). Kept intentionally small: it parses
//! the subset of JSON the fixture format needs (objects, arrays, strings,
//! numbers, booleans, null) with no escape-sequence decoding beyond `\"`,
//! `\\`, and `\n`, and serializes with sorted object keys so that output
//! is byte-for-byte canonical (see the specification's requirement that
//! canonicalized output be comparable across implementations).
//!
//! This crate stays `std`-only by design (see `lib.rs` doc comment), so
//! this module exists instead of taking on `serde_json` as a dependency.

use std::collections::BTreeMap;
use std::fmt;

#[derive(Clone, Debug, PartialEq)]
pub enum Json {
    Null,
    Bool(bool),
    Number(f64),
    String(String),
    Array(Vec<Json>),
    /// `BTreeMap` rather than `HashMap` so iteration order — and thus
    /// serialization — is deterministic and sorted without extra work.
    Object(BTreeMap<String, Json>),
}

impl Json {
    pub fn as_str(&self) -> Option<&str> {
        match self {
            Json::String(s) => Some(s),
            _ => None,
        }
    }

    pub fn as_u64(&self) -> Option<u64> {
        match self {
            Json::Number(n) => Some(*n as u64),
            _ => None,
        }
    }

    pub fn as_bool(&self) -> Option<bool> {
        match self {
            Json::Bool(b) => Some(*b),
            _ => None,
        }
    }

    pub fn as_array(&self) -> Option<&[Json]> {
        match self {
            Json::Array(a) => Some(a),
            _ => None,
        }
    }

    pub fn as_object(&self) -> Option<&BTreeMap<String, Json>> {
        match self {
            Json::Object(o) => Some(o),
            _ => None,
        }
    }

    pub fn get(&self, key: &str) -> Option<&Json> {
        self.as_object().and_then(|o| o.get(key))
    }

    /// Convenience: field lookup with a clear panic message, used by the
    /// fixture runner where a missing required field is a fixture bug.
    pub fn field(&self, key: &str) -> &Json {
        self.get(key)
            .unwrap_or_else(|| panic!("fixture JSON missing required field {:?}", key))
    }
}

impl fmt::Display for Json {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Json::Null => write!(f, "null"),
            Json::Bool(b) => write!(f, "{}", b),
            Json::Number(n) => {
                if n.fract() == 0.0 && n.abs() < 1e15 {
                    write!(f, "{}", *n as i64)
                } else {
                    write!(f, "{}", n)
                }
            }
            Json::String(s) => write!(f, "\"{}\"", escape(s)),
            Json::Array(a) => {
                write!(f, "[")?;
                for (i, v) in a.iter().enumerate() {
                    if i > 0 {
                        write!(f, ",")?;
                    }
                    write!(f, "{}", v)?;
                }
                write!(f, "]")
            }
            Json::Object(o) => {
                write!(f, "{{")?;
                for (i, (k, v)) in o.iter().enumerate() {
                    if i > 0 {
                        write!(f, ",")?;
                    }
                    write!(f, "\"{}\":{}", escape(k), v)?;
                }
                write!(f, "}}")
            }
        }
    }
}

fn escape(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"").replace('\n', "\\n")
}

pub fn parse(input: &str) -> Result<Json, String> {
    let chars: Vec<char> = input.chars().collect();
    let mut pos = 0usize;
    let v = parse_value(&chars, &mut pos)?;
    skip_ws(&chars, &mut pos);
    Ok(v)
}

fn skip_ws(chars: &[char], pos: &mut usize) {
    while *pos < chars.len() && chars[*pos].is_whitespace() {
        *pos += 1;
    }
}

fn parse_value(chars: &[char], pos: &mut usize) -> Result<Json, String> {
    skip_ws(chars, pos);
    if *pos >= chars.len() {
        return Err("unexpected end of input".into());
    }
    match chars[*pos] {
        '{' => parse_object(chars, pos),
        '[' => parse_array(chars, pos),
        '"' => Ok(Json::String(parse_string(chars, pos)?)),
        't' => {
            expect_literal(chars, pos, "true")?;
            Ok(Json::Bool(true))
        }
        'f' => {
            expect_literal(chars, pos, "false")?;
            Ok(Json::Bool(false))
        }
        'n' => {
            expect_literal(chars, pos, "null")?;
            Ok(Json::Null)
        }
        _ => parse_number(chars, pos),
    }
}

fn expect_literal(chars: &[char], pos: &mut usize, lit: &str) -> Result<(), String> {
    for c in lit.chars() {
        if *pos >= chars.len() || chars[*pos] != c {
            return Err(format!("expected literal {:?}", lit));
        }
        *pos += 1;
    }
    Ok(())
}

fn parse_object(chars: &[char], pos: &mut usize) -> Result<Json, String> {
    *pos += 1; // consume '{'
    let mut map = BTreeMap::new();
    skip_ws(chars, pos);
    if *pos < chars.len() && chars[*pos] == '}' {
        *pos += 1;
        return Ok(Json::Object(map));
    }
    loop {
        skip_ws(chars, pos);
        let key = parse_string(chars, pos)?;
        skip_ws(chars, pos);
        if *pos >= chars.len() || chars[*pos] != ':' {
            return Err("expected ':' in object".into());
        }
        *pos += 1;
        let value = parse_value(chars, pos)?;
        map.insert(key, value);
        skip_ws(chars, pos);
        if *pos < chars.len() && chars[*pos] == ',' {
            *pos += 1;
            continue;
        }
        if *pos < chars.len() && chars[*pos] == '}' {
            *pos += 1;
            break;
        }
        return Err("expected ',' or '}' in object".into());
    }
    Ok(Json::Object(map))
}

fn parse_array(chars: &[char], pos: &mut usize) -> Result<Json, String> {
    *pos += 1; // consume '['
    let mut items = Vec::new();
    skip_ws(chars, pos);
    if *pos < chars.len() && chars[*pos] == ']' {
        *pos += 1;
        return Ok(Json::Array(items));
    }
    loop {
        let value = parse_value(chars, pos)?;
        items.push(value);
        skip_ws(chars, pos);
        if *pos < chars.len() && chars[*pos] == ',' {
            *pos += 1;
            continue;
        }
        if *pos < chars.len() && chars[*pos] == ']' {
            *pos += 1;
            break;
        }
        return Err("expected ',' or ']' in array".into());
    }
    Ok(Json::Array(items))
}

fn parse_string(chars: &[char], pos: &mut usize) -> Result<String, String> {
    if chars[*pos] != '"' {
        return Err("expected '\"'".into());
    }
    *pos += 1;
    let mut s = String::new();
    while *pos < chars.len() && chars[*pos] != '"' {
        if chars[*pos] == '\\' && *pos + 1 < chars.len() {
            *pos += 1;
            match chars[*pos] {
                'n' => s.push('\n'),
                't' => s.push('\t'),
                '"' => s.push('"'),
                '\\' => s.push('\\'),
                other => s.push(other),
            }
        } else {
            s.push(chars[*pos]);
        }
        *pos += 1;
    }
    if *pos >= chars.len() {
        return Err("unterminated string".into());
    }
    *pos += 1; // consume closing quote
    Ok(s)
}

fn parse_number(chars: &[char], pos: &mut usize) -> Result<Json, String> {
    let start = *pos;
    if *pos < chars.len() && (chars[*pos] == '-' || chars[*pos] == '+') {
        *pos += 1;
    }
    while *pos < chars.len() && (chars[*pos].is_ascii_digit() || chars[*pos] == '.' || chars[*pos] == 'e' || chars[*pos] == 'E' || chars[*pos] == '-' || chars[*pos] == '+') {
        *pos += 1;
    }
    let s: String = chars[start..*pos].iter().collect();
    s.parse::<f64>().map(Json::Number).map_err(|e| format!("bad number {:?}: {}", s, e))
}
