#include "wire.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { unsigned char *p; size_t len, cap; } Buffer;
static bool grow(Buffer *b,size_t n){if(n>SIZE_MAX-b->len)return false;size_t need=b->len+n;if(need<=b->cap)return true;size_t cap=b->cap?b->cap:128;while(cap<need){if(cap>SIZE_MAX/2){cap=need;break;}cap*=2;}unsigned char*p=realloc(b->p,cap);if(!p)return false;b->p=p;b->cap=cap;return true;}
static bool put(Buffer*b,const void*p,size_t n){if(!grow(b,n))return false;memcpy(b->p+b->len,p,n);b->len+=n;return true;}
static bool u8(Buffer*b,unsigned v){unsigned char x=(unsigned char)v;return put(b,&x,1);}
static bool u32(Buffer*b,uint32_t v){unsigned char x[4]={(unsigned char)(v>>24),(unsigned char)(v>>16),(unsigned char)(v>>8),(unsigned char)v};return put(b,x,4);}
static bool u64(Buffer*b,uint64_t v){unsigned char x[8];for(int i=7;i>=0;i--){x[i]=(unsigned char)v;v>>=8;}return put(b,x,8);}
static bool str(Buffer*b,const char*s){size_t n=strlen(s?s:"");return n<=UINT32_MAX&&u32(b,(uint32_t)n)&&put(b,s?s:"",n);}
static int cmp_id(const void*a,const void*b){uint64_t x=*(const uint64_t*)a,y=*(const uint64_t*)b;return(x>y)-(x<y);}
static int cmp_rule(const void*a,const void*b){return strcmp(*(char*const*)a,*(char*const*)b);}

bool wire_encode(const Arbiter *arb,unsigned char **out,size_t*out_len,char*err){
 Buffer b={0}; static const unsigned char magic[8]={'S','P','H','I','S','T','1',0};
 ObjectId*omega=malloc((arb->omega0.len?arb->omega0.len:1)*sizeof*omega);char**rules=malloc((arb->rules.len?arb->rules.len:1)*sizeof*rules);
 if(!omega||!rules){snprintf(err,128,"allocation failure");free(omega);free(rules);return false;}memcpy(omega,arb->omega0.items,arb->omega0.len*sizeof*omega);memcpy(rules,arb->rules.items,arb->rules.len*sizeof*rules);qsort(omega,arb->omega0.len,sizeof*omega,cmp_id);qsort(rules,arb->rules.len,sizeof*rules,cmp_rule);
 bool ok=arb->omega0.len<=UINT32_MAX&&arb->rules.len<=UINT32_MAX&&arb->history_len<=UINT32_MAX&&put(&b,magic,8)&&u32(&b,(uint32_t)arb->omega0.len);
 for(size_t i=0;ok&&i<arb->omega0.len;i++) ok=u64(&b,omega[i]);
 ok=ok&&u32(&b,(uint32_t)arb->rules.len);
 for(size_t i=0;ok&&i<arb->rules.len;i++) ok=str(&b,rules[i]);
 ok=ok&&u32(&b,(uint32_t)arb->history_len);
 for(size_t i=0;ok&&i<arb->history_len;i++){const Event*e=&arb->history[i];ok=u8(&b,(unsigned)e->kind);if(!ok)break;switch(e->kind){case EV_POP:ok=u64(&b,e->a);break;case EV_REFUSE:ok=u64(&b,e->a)&&u8(&b,e->has_b?1:0)&&(!e->has_b||u64(&b,e->b))&&str(&b,e->reason);break;case EV_BIND:ok=u64(&b,e->a)&&u64(&b,e->b)&&str(&b,e->tag);break;case EV_COLLAPSE:ok=str(&b,e->rule);break;default:ok=false;}}
 free(omega);free(rules);if(!ok){snprintf(err,128,"cannot encode SPHIST/1 envelope");free(b.p);return false;}*out=b.p;*out_len=b.len;return true;
}

typedef struct{const unsigned char*p;size_t len,off;}Reader;
static bool take(Reader*r,size_t n,const unsigned char**out){if(n>r->len-r->off)return false;*out=r->p+r->off;r->off+=n;return true;}
static bool rd8(Reader*r,uint8_t*out){const unsigned char*p;if(!take(r,1,&p))return false;*out=p[0];return true;}
static bool rd32(Reader*r,uint32_t*out){const unsigned char*p;if(!take(r,4,&p))return false;*out=((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];return true;}
static bool rd64(Reader*r,uint64_t*out){const unsigned char*p;if(!take(r,8,&p))return false;uint64_t v=0;for(int i=0;i<8;i++)v=(v<<8)|p[i];*out=v;return true;}
static bool valid_utf8(const unsigned char*s,size_t n){size_t i=0;while(i<n){unsigned c=s[i++];size_t more;if(c<0x80)continue;if(c>=0xc2&&c<=0xdf)more=1;else if(c>=0xe0&&c<=0xef)more=2;else if(c>=0xf0&&c<=0xf4)more=3;else return false;if(i+more>n)return false;for(size_t j=0;j<more;j++)if((s[i+j]&0xc0)!=0x80)return false;if(more==2&&c==0xe0&&s[i]<0xa0)return false;if(more==2&&c==0xed&&s[i]>=0xa0)return false;if(more==3&&c==0xf0&&s[i]<0x90)return false;if(more==3&&c==0xf4&&s[i]>=0x90)return false;i+=more;}return true;}
static bool rdstr(Reader*r,char**out){uint32_t n;const unsigned char*p;if(!rd32(r,&n)||!take(r,n,&p)||!valid_utf8(p,n))return false;char*s=malloc((size_t)n+1);if(!s)return false;memcpy(s,p,n);s[n]=0;*out=s;return true;}

bool wire_decode_replay(const unsigned char*data,size_t len,State*out,char*err){
 Reader r={data,len,0};const unsigned char*p;static const unsigned char magic[8]={'S','P','H','I','S','T','1',0};uint32_t n;ObjectSet omega;objset_init(&omega);
 if(!take(&r,8,&p)||memcmp(p,magic,8)||!rd32(&r,&n)) goto bad;
 uint64_t prev=0;
 for(uint32_t i=0;i<n;i++){uint64_t id;if(!rd64(&r,&id)||(i&&id<=prev))goto bad;objset_add(&omega,id);prev=id;}
 if(!rd32(&r,&n)) goto bad;
 char*prev_rule=NULL;
 for(uint32_t i=0;i<n;i++){char*s=NULL;if(!rdstr(&r,&s)||(prev_rule&&strcmp(prev_rule,s)>=0)){free(s);free(prev_rule);goto bad;}free(prev_rule);prev_rule=s;}
 free(prev_rule);
 state_init(out,&omega);objset_free(&omega);if(!rd32(&r,&n)){state_free(out);goto bad_noomega;}
 for(uint32_t i=0;i<n;i++){uint8_t kind;if(!rd8(&r,&kind)){state_free(out);goto bad_noomega;}Event e={0};uint64_t a,b;char*s=NULL;bool ok=true;switch(kind){case EV_POP:ok=rd64(&r,&a);if(ok)e=event_pop(a);break;case EV_REFUSE:{uint8_t flag;ok=rd64(&r,&a)&&rd8(&r,&flag)&&flag<=1;if(ok&&flag)ok=rd64(&r,&b);if(ok)ok=rdstr(&r,&s);if(ok)e=flag?event_refuse_bind(a,b,s):event_refuse(a,s);free(s);break;}case EV_BIND:ok=rd64(&r,&a)&&rd64(&r,&b)&&rdstr(&r,&s);if(ok)e=event_bind(a,b,s);free(s);break;case EV_COLLAPSE:ok=rdstr(&r,&s);if(ok)e=event_collapse(s);free(s);break;default:ok=false;}if(!ok){state_free(out);goto bad_noomega;}state_apply(out,&e);event_free(&e);}
 if(r.off!=r.len){state_free(out);snprintf(err,128,"trailing bytes in SPHIST/1 envelope");return false;}return true;
bad:objset_free(&omega);bad_noomega:snprintf(err,128,"invalid or truncated SPHIST/1 envelope");return false;
}

void wire_fnv1a64_hex(const unsigned char*data,size_t len,char out[17]){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<len;i++){h^=data[i];h*=UINT64_C(0x100000001b3);}snprintf(out,17,"%016llx",(unsigned long long)h);}
