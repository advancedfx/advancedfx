#!/usr/bin/env python

import sys
import os
import re


readme_content = ''

with open('misc/readme_head.txt', 'r') as readme_file:
    readme_content += readme_file.read()

md_regex = re.compile('---(.*)---(.*)', re.DOTALL | re.MULTILINE)
md_props_regex = re.compile('^\s*([^\:]*)\s*\:\s*(.*)$', re.MULTILINE)

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
        result += m.get('name','') + '\n'
        val = m.get('contributions', None)
        if val is not None:
            result += '  ' + val + '\n'
        val = m.get('date_from', None)
        if val is not None:
            result += '  ' + val
            val2 = m.get('date_to', None)
            if val2 is not None:
                result += ' - ' + val2
            result += '\n'
        val = m.get('links', None)
        if val is not None:
            result += '  ' + val + '\n'
        val = m.get('content', None)
        if val is not None:
            result += '  ' + val.replace('\n', '\n'+' ') + '\n'
    return result

if(0 < len(contrib_members_current)):
    readme_content += '\n' + '\n' + 'Current HLAE Team members:' + '\n'
    readme_content += mk_members_string(contrib_members_current)
if(0 < len(contrib_members_past)):
    readme_content += '\n' + '\n' + 'Past HLAE Team members:' + '\n'
    readme_content += mk_members_string(contrib_members_past)
if(0 < len(contrib_others)):
    readme_content += '\n' + '\n' + 'Other contributors:' + '\n'
    readme_content += mk_members_string(contrib_others)

readme_path = sys.argv[1] + "/readme.txt"

with open(readme_path, 'w') as f:
    f.write(readme_content)
