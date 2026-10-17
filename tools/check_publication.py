"""Check tracked publication files for private paths, identifiers and broken local links."""
import argparse,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SUFFIXES={'.md','.py','.cpp','.h','.ps1','.asm','.def','.manifest','.txt','.json','.gitignore'}
def check():
    result=subprocess.run(['git','ls-files','-z'],cwd=ROOT,capture_output=True,check=True)
    files=[Path(p) for p in result.stdout.decode().split('\0') if p]
    failures=[]
    for relative in files:
        p=ROOT/relative
        if p.suffix.lower() not in SUFFIXES and p.name not in {'.gitignore','.gitattributes','LICENSE'}:
            failures.append(str(relative)+': unexpected file type');continue
        text=p.read_text(encoding='utf-8-sig')
        # Construct patterns so this checker can itself be published.
        forbidden=[r'(?i)[a-z]:[/\\](?:users|documents and settings)[/\\]',r'(?i)/home/[^/ ]+/',r'(?i)'+''.join(['stereo','convolver']),r'(?i)'+''.join(['aar','on']),r'(?i)(?:ghp|github_pat)_[A-Za-z0-9_]{20,}',r'(?i)\{0\.0\.[01]\.00000000\}\.\{[0-9a-f-]+\}']
        for pattern in forbidden:
            if relative.as_posix() != "tools/check_publication.py" and re.search(pattern,text):failures.append(str(relative)+': private identifier/path pattern')
        if p.suffix=='.md':
            for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)',text):
                target=target.split('#')[0].split(' "')[0].strip('<>')
                if not target or re.match(r'^[a-z]+:',target):continue
                from urllib.parse import unquote
                if not (p.parent/unquote(target)).exists():failures.append(str(relative)+': missing link '+target)
    if failures:raise SystemExit('\n'.join(failures))
    print('Publication checks passed: %d tracked files'%len(files))
if __name__=='__main__':check()
