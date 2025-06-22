#!/usr/bin/env python

import sys
import os
import re


tpl_credits = '''# Credits

## Team

### Current
{team}
### Past
{team_past}
## Contributors
{contributors}
## Donors

Thanks to our donors:

* Open Collective  
  https://opencollective.com/advancedfx/

* GitHub  
  https://github.com/sponsors/advancedfx  
  (Money is sent to our OpenCollective account.)

## Additonal Credits

Additional credits are included with e.g. the main download in the readme.txt file [(or see version on github.com)](https://github.com/advancedfx/advancedfx/blob/master/misc/readme_head.txt).
'''

md_regex = re.compile(r'---(.*)---(.*)', re.DOTALL | re.MULTILINE)
md_props_regex = re.compile(r'^\s*([^\:]*)\s*\:\s*(.*)$', re.MULTILINE)

contrib_members_current = []
contrib_members_past = []
contrib_others = []
contrib_dir = os.fspath('deps/release/contrib')
md_filenames = list(filter(lambda x: x.endswith('.md'), os.listdir(contrib_dir)))
for md_filename in md_filenames:
    with open(os.path.join(contrib_dir, md_filename), 'r') as md_file:
        contrib = {}
        md_content = md_file.read()
        md_match = md_regex.match(md_content)
        content_props, content_extra = md_match.groups((1,2))
        content_props = content_props.strip('\r\n')
        content_extra = content_extra.strip('\r\n')
        if(0 < len(content_extra)):
            contrib['content'] = content_extra
        md_props_matches = md_props_regex.findall(content_props)
        member = False
        past = False
        for match in md_props_matches:
            prop_k = match[0].strip("\"")
            prop_v = match[1].strip("\"")
            
            contrib[prop_k] = prop_v

            if 'member' == prop_k:
                member = 'true' == prop_v
            elif 'date_to' == prop_k and 0 < len(prop_v):
                past = True
        
        if member:
            if past:
                contrib_members_past.append(contrib)
            else:
                contrib_members_current.append(contrib)
        else:
            contrib_others.append(contrib)
            
def mk_members_string(members):
    result = ''
    
    for m in sorted(members, key=lambda item: item.get('date_from','') + ' ' + item.get('date_to', '')):
        result += '\n'
        result += '#### ' + m.get('name','') + '\n'
        val = m.get('date_from', None)
        if val is not None:
            result += '\n' + val
            val2 = m.get('date_to', None)
            if val2 is not None:
                result += ' - ' + val2
            result += '\n'
        val = m.get('contributions', None)
        if val is not None:
            result += '\n' + val + '\n'
        val = m.get('content', None)
        if val is not None:
            result += '\n' + val.replace('\n', '\n'+'  ') + '\n'
        val = m.get('links', None)
        if val is not None:
            result += '\n' + val + '\n'
    return result

credits_content = tpl_credits.format(team=mk_members_string(contrib_members_current),team_past=mk_members_string(contrib_members_past),contributors= mk_members_string(contrib_others))

with open('CREDITS.md', 'w') as f:
    f.write(credits_content)
