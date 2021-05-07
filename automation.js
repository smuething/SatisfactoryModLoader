"use strict";
const fs = require("fs");
const path = require("path");

if (process.argv.length < 3) {
    console.error("Usage: node automation.js <SatisfactoryPath> <...ModList>");
    process.exit(1);
}

const ModLoaderRootPath = process.cwd()
const SatisfactoryPath = process.argv[2];
const ModList = process.argv.slice(3);

console.log(`SF Root: ${SatisfactoryPath} ModLoader Root: ${ModLoaderRootPath} Mod List: ${ModList.join(", ")}`);

function CopyFileAndLog(SourceFilePath, DestinationFolder) {
    if (fs.existsSync(SourceFilePath)) {
        let DestinationFile = `${DestinationFolder}/${path.basename(SourceFilePath)}`;
        try {
            fs.copyFileSync(SourceFilePath, DestinationFile);
            console.info(`${SourceFilePath} -> ${DestinationFile}`);
            return true;
        } catch (e) {
            console.error(`Failed to copy file ${SourceFilePath} to ${DestinationFolder}: ${e.message}`);
        }
    }
    return false;
}

let ModsCopied = 0;

for (let ModId of ModList) {
    let SourceFilePathBase = `${ModLoaderRootPath}/Plugins/${ModId}/Binaries/Win64/FactoryGame-${ModId}-Win64-Shipping`;
    let DestinationFolder = `${SatisfactoryPath}/FactoryGame/Mods/${ModId}/Binaries/Win64`;
    if (CopyFileAndLog(`${SourceFilePathBase}.dll`, DestinationFolder)) {
        CopyFileAndLog(`${SourceFilePathBase}.pdb`, DestinationFolder);
        ModsCopied++;
    }
}
console.log(`Done. Copied ${ModsCopied} mod(s).`);

if (ModsCopied !== ModList.length) {
    process.exit(1);
}