/*
* Copyright 2013-2016, Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may
* only be used strictly in accordance with the applicable license terms. 
* By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that
* you have read, and that you agree to comply with and are bound by,
* such license terms.  If you do not agree to be bound by the applicable
* license terms, then you may not retain, install, activate or otherwise
* use the software.
*/

const ROW = "row";
const CELL = "cell";
const TOUCH = "radio";

class Matrix {
    constructor(dimension, tabId) {
        this.tabId = tabId
        this.dimension = dimension;
        this.values = Array.from(Array(this.dimension), () => new Array(this.dimension).fill(0));
        this.touchTimes = -1;
    }

    createMatrix(element) {        
        const table = document.createElement("div");
        table.appendChild(document.createElement("br"));
        table.setAttribute("id", this.tabId);
        table.setAttribute("class", "table");
        for (let irow = 0; irow < this.dimension; irow++) {
            const rowId = `${this.tabId}${ROW}${irow}`;
            const row = document.createElement("div");
            row.setAttribute("id", rowId);
            row.setAttribute("class", "row");
            for (let icell = 0; icell < this.dimension; icell++) {
                const cellId = rowId + CELL + icell.toString();
                const cell = document.createElement("div");
                cell.setAttribute("id", cellId);
                cell.setAttribute("class", "cell");
                cell.innerHTML = 1;
                row.appendChild(cell);
            }
            table.appendChild(row);
        }
        element.appendChild(table);
    }

    setDiagonalValue() {
        for (let i = 0; i < this.dimension; i++) {
            const diagId = `${this.tabId}${ROW}${i}${CELL}${i}`;
            this.values[i][i] = this.touchTimes !== -1 ? 32767 * this.touchTimes : 32767;
            this.setValue(32767, diagId);
        }
    }

    setValue(value, cellId) {
        const getHexColor = (number) => {
            return `hsl(${Math.round((32767 - number) / 32767 * 240)}, 50%, 75%)`;
        }
        const x = document.getElementById(cellId);
        x.style.backgroundColor = getHexColor(value);
        x.style.color = getHexColor(value);
        x.title = value;
        x.innerHTML = value;
    }

    saveValues(values, fileName) {
        pcm.LocalFileOpen(fileName, "w").then(response => {
            const descriptor = response.data;
            pcm.LocalFileWriteString(descriptor, values, false).then(() => {
                pcm.LocalFileClose(descriptor).then(() => console.log(`values to ${fileName} dumped`)).catch(err => console.log("Error while closing:", JSON.stringify(err)));
            }).catch(err => console.log(`Error ${JSON.stringify(err)} while writing to file: ${fileName}`));
        }).catch(err => console.log(`Error ${err} while opening ${fileName}`));
    }

    readValues(fileName, callback) {
        pcm.LocalFileOpen(fileName, "r").then((response) => {
            const fileDescriptor = response.data;
            pcm.LocalFileReadString(fileDescriptor, 0x7FFFFFFF, false).then((xd) => {
                pcm.LocalFileClose(fileDescriptor).then(() => {
                    callback(xd.data);
                }).catch(err => console.log(`Error ${err} while closing ${fileName}`));
            }).catch(err => console.log(`Error ${err} while reading ${fileName}`));
        }).catch(err => console.log(`Error ${err} while opening ${fileName}`));

    }
}