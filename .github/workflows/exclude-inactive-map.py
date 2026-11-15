from pathlib import Path

path = Path('app/src/main/java/com/lurion/launcher/activity/InstallActivity.java')
text = path.read_text(encoding='utf-8')
needle = '                entryName = entryName.substring("files/".length());\n'
patch = needle + '''                // Urbano cache optimization: cl1 is the active map in mapas/.troca.json.
                // Do not extract the inactive alternate map _rio; keep the ZIP and its
                // SHA-256 validation unchanged, and preserve all shared game files.
                String mapPath = entryName.endsWith("/")
                        ? entryName.substring(0, entryName.length() - 1) : entryName;
                if (mapPath.equals("mapas/_rio") || mapPath.startsWith("mapas/_rio/")) continue;
'''
if text.count(needle) != 1:
    raise SystemExit(f'Expected exactly one extraction path to patch, found {text.count(needle)}')
path.write_text(text.replace(needle, patch, 1), encoding='utf-8')
print('Installer configured to skip only inactive mapas/_rio during extraction; ZIP SHA-256 validation unchanged.')
