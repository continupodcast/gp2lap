"""Maintain shipped CFG defaults without discarding explicit legacy feature settings."""
import re
ZERO_KEYS={'jamtextureextramb','extratracksegments','extratrackfilekb','pitspeedkmh','ailowhplaunchfix','aibankingroll'}
LEGACY_SECTIONS={'textures','track','pit','ai','season'}
RCR_KEYS=['svgarcr1','svgarcr2','svgarcr3']
def clean_cfg(text):
    lines=text.splitlines()
    active={m.group(1).lower():m.group(2).split(';',1)[0].strip() for line in lines if (m:=re.match(r'^\s*([A-Za-z]\w*)\s*=\s*(.*)',line))}
    kept=[];activated=set()
    for line in lines:
        m=re.match(r'^\s*(;?)\s*([A-Za-z]\w*)\s*=\s*(.*)',line)
        if m:
            commented,key,value=m.group(1),m.group(2).lower(),m.group(3).split(';',1)[0].strip()
            if (key in ZERO_KEYS and (commented or value=='0')) or (key=='ailaunchfadekmh' and (commented or (value=='128' and active.get('ailowhplaunchfix','0')=='0'))) or (key=='seasonoverrides' and (commented or value in ('""',"''"))):continue
            if key in RCR_KEYS:
                if key in activated or (commented and key in active):continue
                line=re.sub(r'^(\s*);\s*',r'\1',line);activated.add(key)
        kept.append(line)
    # Drop only empty legacy sections; preserve any explicit enabled or unknown key.
    out=[];block=[];section=''
    def flush():
        if section in LEGACY_SECTIONS and not any(re.match(r'^\s*[^;\s\[]+\s*=',s) for s in block[1:]):return
        out.extend(block)
    for line in kept:
        m=re.match(r'^\s*\[([^\]]+)\]',line)
        if m:flush();block=[line];section=m.group(1).lower()
        else:block.append(line)
    flush()
    missing=[k for k in RCR_KEYS if k not in activated]
    if missing:out+=['','[Rcr]']+['SvgaRcr'+k[-1]+' = 9999000' for k in missing]
    return '\n'.join(out).rstrip()+'\n'
