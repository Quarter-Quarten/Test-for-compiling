from pathlib import Path
for f in Path('.').rglob('*.o'):
    f.unlink()
for f in Path('.').rglob('*.os'):
    f.unlink()
for f in Path('.').rglob('*.cpp.bak'):
    f.unlink()
for f in Path('.').rglob('*.hpp.bak'):
    f.unlink()