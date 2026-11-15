from pathlib import Path
import os

path = Path('app/build.gradle')
text = path.read_text(encoding='utf-8')
if 'signingConfigs {' in text and 'urbanoRelease' in text:
    raise SystemExit('app/build.gradle already contains urbanoRelease signing; review instead of double-patching')

# Keep the existing app id and project config; alter only the version and release signing.
if 'versionCode 109' not in text:
    raise SystemExit('Expected versionCode 109 not found; refusing to guess or overwrite app/build.gradle')
text = text.replace('versionCode 109', 'versionCode Integer.parseInt(System.getenv("URBANO_VERSION_CODE"))', 1)
if 'versionName "migracao_agrupada_109"' not in text:
    raise SystemExit('Expected versionName migracao_agrupada_109 not found; refusing to guess')
text = text.replace('versionName "migracao_agrupada_109"', 'versionName System.getenv("URBANO_VERSION_NAME")', 1)
needle = '    buildTypes {\n'
if text.count(needle) != 1:
    raise SystemExit('Could not locate unique Android buildTypes block')
text = text.replace(needle, '''    signingConfigs {
        urbanoRelease {
            storeFile file(System.getenv("URBANO_KEYSTORE_PATH"))
            storeType "PKCS12"
            storePassword System.getenv("URBANO_KEYSTORE_PASSWORD")
            keyAlias System.getenv("URBANO_KEY_ALIAS")
            keyPassword System.getenv("URBANO_KEY_PASSWORD")
        }
    }

    buildTypes {
''', 1)
needle = '        release {\n'
if text.count(needle) != 1:
    raise SystemExit('Could not locate unique release buildType')
text = text.replace(needle, '        release {\n            signingConfig signingConfigs.urbanoRelease\n', 1)
path.write_text(text, encoding='utf-8')
print('Configured app/build.gradle for persistent Urbano release signing')
