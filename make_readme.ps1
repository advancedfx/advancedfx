$contrib_members_current = @()
$contrib_members_past = @()
$contrib_others = @()

Get-ChildItem ".\contrib\" -Filter *.md | Foreach-Object {
	$content = Get-Content $_.FullName -Raw
	$matches = $content | Select-String '(?sm)---(.*)---(.*)'
	
	$content_props = $matches.Matches.Groups[1].Value.Trim(" `n")
	$content_extra = $matches.Matches.Groups[2].Value.Trim(" `n")
	
	$matches = $content_props | Select-String -AllMatches '(?m)^\s*([^\:]*)\s*\:\s*(.*)$'
	
	$contrib = @{}
	if(0 -lt $content_extra.Length) { $contrib.content = $content_extra }
	
	$member = 'false'
	$past = 'false'
	
	foreach($m in $matches.Matches) {
		$m_k = $m.Groups[1].Value.Trim('"')
		$m_v = $m.Groups[2].Value.Trim('"')
		
		$contrib.Add($m_k,$m_v)
		
		if('member' -eq $m_k) {
			$member = $m_v
		}
		elseif('date_to' -eq $m_k -and 0 -lt $m_v.Length)
		{
			$past = 'true'
		}
	}
	
	if($member -eq 'true') {
		if($past -eq 'true') {
			$contrib_members_past += $contrib
		}
		else {
			$contrib_members_current += $contrib
		}
	}
	else {
		$contrib_others += $contrib
	}
}

$readme = Get-Content '.\misc\readme_head.txt' -Raw

function mk_members_string($members)
{
	$result = ''

	foreach($m in $members | Sort-Object { [string]$_.date_from + '_' +[string]$_.date_to } )
	{
		$result += "`n"
		$result += $m.name +"`n"
		if($m.contributions -ne $null) {
			$result += '  ' + $m.contributions + "`n"
		}
		if($m.date_from -ne $null) {
			$result += '  ' + $m.date_from 
			if($m.date_to -ne $null) { $result += ' - ' + $m.date_to }
			$result += "`n"
		}
		if($m.links -ne $null) {
			$result += '  ' + $m.links + "`n"
		}
		if($m.content -ne $null) {
			$result += '  ' + $m.content.Replace("`n", "`n  ") + "`n"
		}
	}
	
	return $result
}

if(0 -lt $contrib_members_current.Length)
{
	$readme += "`n" + "`n" + 'Current HLAE Team members:' + "`n"
	$readme += mk_members_string($contrib_members_current)
}

if(0 -lt $contrib_members_past.Length)
{
	$readme += "`n" + "`n" + 'Past HLAE Team members:' + "`n"
	$readme += mk_members_string($contrib_members_past)
}

if(0 -lt $contrib_others.Length)
{
	$readme += "`n" + "`n" + 'Other contributors:' + "`n"
	$readme += mk_members_string($contrib_others)
}

Set-Content -Path '.\build\Release\bin\readme.txt' -Value $readme