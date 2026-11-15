from pathlib import Path
import os
import re

path = Path('app/build.gradle')
text = path.read_text(encoding='utf-8')
version_code = os.environ.get('URBANO_VERSION_CODE')
version_name = os.environ.get('URBANO_VERSION_NAME')
required = ('URBANO_KEYSTORE_PATH', 'URBANO_KEYSTORE_PASSWORD', 'URBANO_KEY_ALIAS', 'URBANO_KEY_PASSWORD')
missing = [key for key in required if not os.environ.get(key)]
if not version_code or not version_name or missing:
    raise SystemExit(f'Missing signing/version environment: {missing}')
if 'signingConfigs {' in text and 'urbanoRelease' in text:
    raise SystemExit('app/build.gradle already has urbanoRelease signing; refusing to duplicate it')
code_re = re.compile(r'(?m)^(\s*versionCode\s+)\d+(\s*)$')
name_re = re.compile(r'(?m)^(\s*versionName\s+)["\'][^"\']+["\'](\s*)$')
if not code_re.search(text) or not name_re.search(text):
    raise SystemExit('Could not identify literal versionCode/versionName in app/build.gradle')
text = code_re.sub(lambda m: f'{m.group(1)}{int(version_code)}{m.group(2)}', text, count=1)
text = name_re.sub(lambda m: f'{m.group(1)}"{version_name}"{m.group(2)}', text, count=1)
needle = '    buildTypes {\n'
if text.count(needle) != 1:
    raise SystemExit('Could not find unique buildTypes block')
signing = '''    signingConfigs {
        urbanoRelease {
            storeFile file(System.getenv("URBANO_KEYSTORE_PATH"))
            storeType "PKCS12"
            storePassword System.getenv("URBANO_KEYSTORE_PASSWORD")
            keyAlias System.getenv("URBANO_KEY_ALIAS")
            keyPassword System.getenv("URBANO_KEY_PASSWORD")
        }
    }

'''
text = text.replace(needle, signing + needle, 1)
release = '        release {\n'
if text.count(release) != 1:
    raise SystemExit('Could not find unique release buildType')
text = text.replace(release, release + '            signingConfig signingConfigs.urbanoRelease\n', 1)
path.write_text(text, encoding='utf-8')
print(f'Configured signing and versionCode={version_code}, versionName={version_name}')
