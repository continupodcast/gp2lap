from pathlib import Path
import struct
root=Path(__file__).resolve().parents[1]
exe=(root.parents[1]/'upload/GP2.EXE').read_bytes()
def raw(a,n):return exe[0x88254+a-0x10000:0x88254+a-0x10000+n]
for a in [0x71358,0x6d7a8]:
 assert raw(a,1)==b'\xe8'
 assert a+5+struct.unpack('<i',raw(a+1,4))[0]==0x7120c
s=(root/'src/frankasm.inc').read_text()
a=s[s.index('_F1NativeVisibilityWrapped proc near'):s.index('_F1NativeTextWrapped proc near')]
assert a.index('_fpF1NativeHiddenAt')<a.index('_fpF1NativeVisibilityOriginal')
assert 'and     dword ptr [esp+32]' not in a
assert 'call    dword ptr ds:_fpF1NativeVisibilityOriginal' in a
assert 'jz      F1VisibilityCompose' in a
assert 'mov     dword ptr ds:_f1NativeSuppressPixels,0' in a
x=s[s.index('_F1NativePixelsWrapped proc near'):s.index('_F1NativeCaptionWrapped proc near')]
assert 'mov     dword ptr ds:_f1NativeSuppressPixels,0' in x
b=s[s.index('_F1NativeTextWrapped proc near'):s.index('_F1NativePixelsWrapped proc near')]
assert 'call    dword ptr ds:_fpF1NativeTextOriginal' in b
assert b.index('_f1NativeSuppressPixels,eax')<b.index('_fpF1NativeTextOriginal')<b.index('_f1NativeSuppressPixels,0')
c=(root/'src/f1native.c').read_text()
assert 'pixelCalls[]={0x71358,0x6d7a8}' in c
print('PASS: cached/new pixel-copy signatures, suppression selected before visibility, native branch flags retained, generic composition executes and suppression resets.')
