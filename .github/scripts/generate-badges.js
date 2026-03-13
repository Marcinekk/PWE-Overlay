import fs from 'fs';
import fetch from 'node-fetch';

const versions = JSON.parse(fs.readFileSync('versions.json', 'utf8'));
const badgesDir = 'assets/badges';
if (!fs.existsSync(badgesDir)) fs.mkdirSync(badgesDir, { recursive: true });

async function createBadge(game, version) {
    const url = `https://img.shields.io/badge/${game}-${version}-blue.svg`;
    const res = await fetch(url);
    const svg = await res.text();
    fs.writeFileSync(`${badgesDir}/${game}.svg`, svg, 'utf8');
}

await Promise.all(
    Object.entries(versions).map(([game, version]) => createBadge(game, version))
);

console.log('Badges updated!');