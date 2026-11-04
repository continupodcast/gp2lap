from pathlib import Path
import sys,unittest,json,subprocess
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
from cfg_defaults import clean_cfg
class Defaults(unittest.TestCase):
 def test_shipped_defaults_and_safe_native_pit_default(self):
  for name in ['release/GP2LAP.CFG','build/gp2lap.cfg']:
   text=(ROOT/name).read_text();assert clean_cfg(text)==text
   for k in ['JamTextureExtraMB','ExtraTrackSegments','ExtraTrackFileKB','PitSpeedKmh','AILowHpLaunchFix','AILaunchFadeKmh','AIBankingRoll','SeasonOverrides']:assert k not in text,k
   for n in range(1,4):assert '\nSvgaRcr'+str(n)+' = 9999000' in text
  assert 'unsigned long PitSpeedKmh = 0;' in (ROOT/'src/timing/convert.inc').read_text()
 def test_legacy_cleanup_preserves_active_settings(self):
  old='[AI]\nAILowHpLaunchFix = 0\nAILaunchFadeKmh = 128\nAIBankingRoll = 0\n[Season]\n;SeasonOverrides = "override.cfg"\n[Rcr]\n;SvgaRcr1 = 9999000\nSvgaRcr2 = 1234\n;SvgaRcr2 = 9999000\n;SvgaRcr3 = 9999000\n[Keep]\nUnknown = 4\n'
  clean=clean_cfg(old);assert '[AI]' not in clean and '[Season]' not in clean;assert clean.count('SvgaRcr2')==1 and 'SvgaRcr2 = 1234' in clean;assert 'Unknown = 4' in clean
  active='[AI]\nAILowHpLaunchFix = 1\nAILaunchFadeKmh = 128\n[Pit]\nPitSpeedKmh = 60\n[Season]\nSeasonOverrides = "custom.cfg"\n'
  fixed=clean_cfg(active)
  for line in active.splitlines():assert line in fixed
  assert clean_cfg(fixed)==fixed
 def test_editor_and_builder_agree(self):
  html=ROOT/'modules/hud-editor/EDITOR.html'
  if not html.exists():self.skipTest('editor module not selected')
  fixtures=['[AI]\nAILowHpLaunchFix = 0\nAILaunchFadeKmh = 128\n[Pit]\nPitSpeedKmh = 0\n[Season]\n;SeasonOverrides = "override.cfg"\n[Rcr]\n;SvgaRcr1 = 9999000\n;SvgaRcr2 = 9999000\n;SvgaRcr3 = 9999000\n','[AI]\nAILowHpLaunchFix = 1\nAILaunchFadeKmh = 128\n[Pit]\nPitSpeedKmh = 60\n[Rcr]\nSvgaRcr1 = 42\n;SvgaRcr1 = 9999000\n','[Unknown]\nCustom = 8\n']
  script="const fs=require('fs'),vm=require('vm');const h=fs.readFileSync(process.argv[1],'utf8');const f=h.slice(h.indexOf('function cleanCfgDefaults('),h.indexOf('function cfg(){'));const c={};vm.createContext(c);vm.runInContext(f,c);process.stdout.write(JSON.stringify(JSON.parse(fs.readFileSync(0,'utf8')).map(c.cleanCfgDefaults)));"
  actual=json.loads(subprocess.check_output(['node','-e',script,str(html)],input=json.dumps(fixtures),text=True))
  meaningful=lambda text:[line for line in text.splitlines() if line.strip()]
  assert [meaningful(s) for s in actual]==[meaningful(clean_cfg(s)) for s in fixtures]
if __name__=='__main__':unittest.main()
