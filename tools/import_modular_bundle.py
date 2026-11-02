#!/usr/bin/env python3
"""Create a NEW local repository containing every modular branch. Does not push."""
import argparse,glob,os,pathlib,shutil,subprocess,sys

def find_git():
    git=shutil.which('git')
    if git:return git
    candidates=[]
    if os.name=='nt':
        candidates=glob.glob(str(pathlib.Path(os.environ.get('LOCALAPPDATA',''))/'GitHubDesktop/app-*/resources/app/git/cmd/git.exe'))
    else:candidates=['/Applications/GitHub Desktop.app/Contents/Resources/app/git/bin/git']
    for path in sorted(candidates,reverse=True):
        if pathlib.Path(path).is_file():return path
    raise SystemExit('Git was not found. Run this script from a terminal with Git available.')
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('destination',type=pathlib.Path);p.add_argument('--bundle',type=pathlib.Path,default=pathlib.Path(__file__).resolve().parent/'GP2Lap-v0.8-modular.bundle');args=p.parse_args()
    bundle=args.bundle.resolve();dest=args.destination.resolve()
    if not bundle.is_file():raise SystemExit('Bundle not found: '+str(bundle))
    if dest.exists():raise SystemExit('Choose a new directory; destination already exists: '+str(dest))
    git=find_git();subprocess.run([git,'clone','--branch','modular/integrated',str(bundle),str(dest)],check=True)
    def run(*args):return subprocess.check_output([git,*args],cwd=dest,text=True).strip()
    refs=run('for-each-ref','--format=%(refname:short)','refs/remotes/origin/').splitlines()
    for ref in refs:
        if ref=='origin/HEAD':continue
        local=ref[len('origin/'):]
        if local=='modular/integrated':continue
        run('branch','--track',local,ref)
    run('remote','set-url','origin','https://github.com/continupodcast/gp2lap.git')
    print('Created repository: '+str(dest))
    print(run('branch','--format=%(refname:short)'))
    print('Open GitHub Desktop > File > Add Local Repository and select this directory.')
    print('Branches are local. Nothing has been pushed or published.')
if __name__=='__main__':main()
