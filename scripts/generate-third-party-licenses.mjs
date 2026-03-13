import crypto from 'node:crypto';
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import { fileURLToPath } from 'node:url';

function repoRoot() {
  const filename = fileURLToPath(import.meta.url);
  return path.resolve(path.dirname(filename), '..');
}

async function exists(p) {
  try {
    await fs.access(p);
    return true;
  } catch {
    return false;
  }
}

function toPosix(p) {
  return p.split(path.sep).join('/');
}

function normalizeRepoUrl(repository) {
  if (!repository) return null;
  if (typeof repository === 'string') return repository;
  if (typeof repository === 'object' && repository.url) return repository.url;
  return null;
}

function normalizeAuthor(author) {
  if (!author) return null;
  if (typeof author === 'string') return author;
  if (typeof author === 'object') {
    const parts = [author.name, author.email && `<${author.email}>`, author.url && `(${author.url})`].filter(Boolean);
    return parts.length ? parts.join(' ') : null;
  }
  return null;
}

function normalizeLicense(license, licenses) {
  if (typeof license === 'string') return license;
  if (typeof license === 'object' && license.type) return license.type;
  if (Array.isArray(licenses)) {
    const types = licenses
      .map((l) => (typeof l === 'string' ? l : l?.type))
      .filter(Boolean);
    if (types.length) return types.join(' OR ');
  }
  return null;
}

function escapeMarkdownTableCell(value) {
  if (!value) return '';
  return String(value).replace(/\r?\n/g, ' ').replaceAll('|', '\\|');
}

function pushFencedText(lines, text) {
  const normalized = String(text ?? '').trimEnd();
  const backtickRuns = normalized.match(/`+/g) ?? [];
  const maxRun = backtickRuns.reduce((m, s) => Math.max(m, s.length), 0);
  const fence = '`'.repeat(Math.max(3, maxRun + 1));
  lines.push(`${fence}text`);
  lines.push(normalized);
  lines.push(fence);
}

async function sha256(text) {
  return crypto.createHash('sha256').update(text).digest('hex');
}

async function readLicenseFileText(pkgDir) {
  const candidates = [
    'LICENSE',
    'LICENSE.md',
    'LICENSE.txt',
    'LICENCE',
    'LICENCE.md',
    'LICENCE.txt',
    'COPYING',
    'COPYING.md',
    'COPYING.txt',
    'NOTICE',
    'NOTICE.md',
    'NOTICE.txt',
  ];

  for (const name of candidates) {
    const p = path.join(pkgDir, name);
    if (await exists(p)) {
      const text = await fs.readFile(p, 'utf8');
      return { file: name, text };
    }
  }
  return null;
}

async function scanNodeModules(nodeModulesDir) {
  const packages = [];
  const visitedDirs = new Set();
  const queue = [nodeModulesDir];

  while (queue.length) {
    const current = queue.shift();
    if (!current) continue;
    const real = await fs.realpath(current).catch(() => current);
    if (visitedDirs.has(real)) continue;
    visitedDirs.add(real);

    if (!(await exists(current))) continue;
    const entries = await fs.readdir(current, { withFileTypes: true }).catch(() => []);

    for (const entry of entries) {
      if (!entry.isDirectory()) continue;
      if (entry.name === '.bin') continue;

      if (entry.name.startsWith('@')) {
        const scopeDir = path.join(current, entry.name);
        const scoped = await fs.readdir(scopeDir, { withFileTypes: true }).catch(() => []);
        for (const scopedEntry of scoped) {
          if (!scopedEntry.isDirectory()) continue;
          const pkgDir = path.join(scopeDir, scopedEntry.name);
          packages.push(pkgDir);
          const nested = path.join(pkgDir, 'node_modules');
          if (await exists(nested)) queue.push(nested);
        }
        continue;
      }

      const pkgDir = path.join(current, entry.name);
      packages.push(pkgDir);
      const nested = path.join(pkgDir, 'node_modules');
      if (await exists(nested)) queue.push(nested);
    }
  }

  return packages;
}

async function readPackageInfo(pkgDir, rootDir) {
  const pkgJsonPath = path.join(pkgDir, 'package.json');
  if (!(await exists(pkgJsonPath))) return null;

  let pkg;
  try {
    pkg = JSON.parse(await fs.readFile(pkgJsonPath, 'utf8'));
  } catch {
    return null;
  }

  if (!pkg?.name || !pkg?.version) return null;

  const license = normalizeLicense(pkg.license, pkg.licenses);
  const repo = normalizeRepoUrl(pkg.repository);
  const homepage = typeof pkg.homepage === 'string' ? pkg.homepage : null;
  const author = normalizeAuthor(pkg.author);

  const licenseFile = await readLicenseFileText(pkgDir);
  const relDir = toPosix(path.relative(rootDir, pkgDir));

  return {
    name: pkg.name,
    version: pkg.version,
    license,
    repository: repo,
    homepage,
    author,
    directory: relDir,
    licenseFile: licenseFile?.file ?? null,
    licenseText: licenseFile?.text ?? null,
  };
}

async function readTextFileIfExists(p) {
  if (!(await exists(p))) return null;
  return fs.readFile(p, 'utf8');
}

async function main() {
  const root = repoRoot();
  const args = process.argv.slice(2);
  const outputPathArgIndex = args.findIndex((a) => a === '--output');
  const outputPath = outputPathArgIndex >= 0 ? args[outputPathArgIndex + 1] : null;

  const nowIso = new Date().toISOString();

  const frontendNodeModules = path.join(root, 'frontend', 'node_modules');
  const frontendPackageJson = path.join(root, 'frontend', 'package.json');
  const hasFrontend = await exists(frontendPackageJson);

  const packageInfos = [];
  if (hasFrontend && (await exists(frontendNodeModules))) {
    const pkgDirs = await scanNodeModules(frontendNodeModules);
    const byKey = new Map();
    for (const pkgDir of pkgDirs) {
      const info = await readPackageInfo(pkgDir, root);
      if (!info) continue;
      const key = `${info.name}@${info.version}`;
      if (!byKey.has(key)) byKey.set(key, info);
    }
    packageInfos.push(...byKey.values());
  }

  packageInfos.sort((a, b) => {
    const nameCmp = a.name.localeCompare(b.name);
    if (nameCmp !== 0) return nameCmp;
    return a.version.localeCompare(b.version);
  });

  const licenseTextsByHash = new Map();
  for (const pkg of packageInfos) {
    if (!pkg.licenseText) continue;
    const hash = await sha256(pkg.licenseText);
    if (!licenseTextsByHash.has(hash)) {
      licenseTextsByHash.set(hash, { hash, text: pkg.licenseText, sources: [] });
    }
    licenseTextsByHash.get(hash).sources.push(`${pkg.name}@${pkg.version}`);
  }

  const frameworkLicense = await readTextFileIfExists(path.join(root, 'Framework', 'LICENSE'));
  const scsSdkLicense = await readTextFileIfExists(path.join(root, 'Framework', 'vendor', 'scs-sdk', 'sdk_license.txt'));
  const webview2License = await readTextFileIfExists(path.join(root, 'backend', 'build', '_deps', 'webview2-src', 'LICENSE.txt'));
  const webview2Notice = await readTextFileIfExists(path.join(root, 'backend', 'build', '_deps', 'webview2-src', 'NOTICE.txt'));

  const stbFiles = [
    path.join(root, 'Framework', 'vendor', 'stb_lib', 'stb_image.h'),
    path.join(root, 'Framework', 'vendor', 'stb_lib', 'stb_image_write.h'),
    path.join(root, 'Framework', 'vendor', 'stb_lib', 'stb_image_resize2.h'),
    path.join(root, 'Framework', 'vendor', 'stb_lib', 'stb_easy_font.h'),
  ];

  const stbLicenseSnippets = [];
  for (const f of stbFiles) {
    if (!(await exists(f))) continue;
    const content = await fs.readFile(f, 'utf8');
    const head = content.split(/\r?\n/).slice(0, 120).join('\n');
    const markerIndex = head.toLowerCase().indexOf('license');
    stbLicenseSnippets.push({
      file: toPosix(path.relative(root, f)),
      head,
      markerIndex,
    });
  }

  const hidapiHeader = await readTextFileIfExists(path.join(root, 'backend', 'External', 'hidapi', 'include', 'hidapi.h'));
  const hidapiSnippet = hidapiHeader ? hidapiHeader.split(/\r?\n/).slice(0, 40).join('\n') : null;

  const lines = [];
  lines.push('# THIRD_PARTY_LICENSES');
  lines.push('');
  lines.push(`Generated: ${nowIso}`);
  lines.push('');
  lines.push('This file lists third-party software licenses used by or bundled with this repository.');
  lines.push('');
  lines.push('## JavaScript / Node.js (frontend)');
  lines.push('');
  if (!hasFrontend) {
    lines.push('_Missing `frontend/package.json`._');
  } else if (!packageInfos.length) {
    lines.push('_No dependencies found in `frontend/node_modules`. Install dependencies and re-generate._');
  } else {
    lines.push('| Package | Version | License | Homepage / repository | Source |');
    lines.push('|---|---:|---|---|---|');
    for (const pkg of packageInfos) {
      const url = escapeMarkdownTableCell(pkg.homepage ?? pkg.repository ?? '');
      const lic = pkg.license ?? '';
      lines.push(`| \`${pkg.name}\` | \`${pkg.version}\` | \`${lic}\` | ${url} | \`${pkg.directory}\` |`);
    }
    lines.push('');
    lines.push('_Note: this list includes all packages present in `node_modules` (including dev-tooling dependencies), because the frontend is built as part of the build process._');
    lines.push('');
  }

  lines.push('## Bundled components (vendor / submodule / build)');
  lines.push('');
  lines.push('- `Framework` (git submodule) — license: Apache-2.0 — `Framework/LICENSE`');
  lines.push('- `Framework/vendor/scs-sdk` — license (MIT-like): `Framework/vendor/scs-sdk/sdk_license.txt`');
  lines.push('- `Framework/vendor/stb_lib` — license is embedded in file headers (stb) — `Framework/vendor/stb_lib/*`');
  lines.push('- `backend/External/hidapi` — HIDAPI (multi-license) — see header `backend/External/hidapi/include/hidapi.h`');
  lines.push('- `backend/build/_deps/webview2-src` — Microsoft.Web.WebView2 (downloaded via CMake FetchContent) — `backend/build/_deps/webview2-src/LICENSE.txt` and `NOTICE.txt`');
  lines.push('');

  lines.push('## License texts');
  lines.push('');
  lines.push('### SPF-Framework (Apache-2.0)');
  lines.push('');
  if (frameworkLicense) {
    pushFencedText(lines, frameworkLicense);
  } else {
    lines.push('_Missing `Framework/LICENSE`._');
  }
  lines.push('');

  lines.push('### SCS SDK (MIT-like)');
  lines.push('');
  if (scsSdkLicense) {
    pushFencedText(lines, scsSdkLicense);
  } else {
    lines.push('_Missing `Framework/vendor/scs-sdk/sdk_license.txt`._');
  }
  lines.push('');

  lines.push('### Microsoft.Web.WebView2 (LICENSE + NOTICE)');
  lines.push('');
  if (webview2License) {
    lines.push('**LICENSE.txt**');
    lines.push('');
    pushFencedText(lines, webview2License);
  } else {
    lines.push('_Missing `backend/build/_deps/webview2-src/LICENSE.txt` (it appears after CMake downloads the package)._');
  }
  lines.push('');
  if (webview2Notice) {
    lines.push('**NOTICE.txt**');
    lines.push('');
    pushFencedText(lines, webview2Notice);
  } else {
    lines.push('_Missing `backend/build/_deps/webview2-src/NOTICE.txt` (it appears after CMake downloads the package)._');
  }
  lines.push('');

  lines.push('### stb (file headers)');
  lines.push('');
  if (!stbLicenseSnippets.length) {
    lines.push('_No stb files found in `Framework/vendor/stb_lib`._');
  } else {
    lines.push('Below is the beginning of each stb file (it contains a license section in comments).');
    lines.push('');
    for (const s of stbLicenseSnippets) {
      lines.push(`**${s.file}**`);
      lines.push('');
      pushFencedText(lines, s.head);
      lines.push('');
    }
  }

  lines.push('### HIDAPI (header notice)');
  lines.push('');
  if (hidapiSnippet) {
    lines.push('This repository does not include the `LICENSE*.txt` files referenced by HIDAPI (full license terms are referenced upstream). Header excerpt:');
    lines.push('');
    pushFencedText(lines, hidapiSnippet);
  } else {
    lines.push('_Missing `backend/External/hidapi/include/hidapi.h`._');
  }
  lines.push('');

  lines.push('### npm package licenses (from `frontend/node_modules`)');
  lines.push('');
  if (!licenseTextsByHash.size) {
    lines.push('_No license texts found in npm packages (no LICENSE/NOTICE/COPYING files found in `node_modules`)._');
  } else {
    const blocks = [...licenseTextsByHash.values()].sort((a, b) => a.hash.localeCompare(b.hash));
    for (const block of blocks) {
      const srcList = block.sources.sort((a, b) => a.localeCompare(b)).slice(0, 30);
      const more = block.sources.length > srcList.length ? ` (+${block.sources.length - srcList.length} more)` : '';
      lines.push(`**Sources:** ${srcList.map((s) => `\`${s}\``).join(', ')}${more}`);
      lines.push('');
      pushFencedText(lines, block.text);
      lines.push('');
    }
  }

  lines.push('## Regenerating');
  lines.push('');
  lines.push('1) Ensure `frontend/node_modules` exists (e.g. `npm --prefix frontend install`).');
  lines.push('2) (Optional) To include Microsoft.Web.WebView2 license files, run CMake so that `backend/build/_deps/webview2-src` exists (e.g. `cmake -S backend -B backend/build`).');
  lines.push('3) Run: `node scripts/generate-third-party-licenses.mjs --output THIRD_PARTY_LICENSES.md`.');
  lines.push('');

  const out = `${lines.join('\n')}\n`;
  if (outputPath) {
    const absOut = path.isAbsolute(outputPath) ? outputPath : path.join(root, outputPath);
    await fs.writeFile(absOut, out, 'utf8');
    return;
  }
  process.stdout.write(out);
}

await main();
