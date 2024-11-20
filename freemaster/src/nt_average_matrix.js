/*
 * Copyright 2022, 2024 NXP
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

/* NT cross talk average class */
class AverageMatrix extends Matrix {
    constructor(dimension, tabId) {
        super(dimension, tabId);
    }

    createMatrix(element) {
        const header = document.createElement("h2");
        header.innerHTML = "Average calibration matrix";
        header.style.textAlign = "center";
        header.style.color = "#0A87EF";
        header.style.textTransform = "uppercase";
        element.appendChild(header);
        element.appendChild(document.createElement("br"));
        element.appendChild(document.createElement("br"));
        element.appendChild(document.createElement("br"));
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
            const f = document.createElement("div");
            f.setAttribute("class", "cell");
            f.setAttribute("id", `${rowId}EL${irow}`);
            f.innerHTML = `ELECTRODE ${irow}`;
            f.style.color = "#00A6FF";
            f.style.fontSize = "11px";
            f.style.borderColor = "white";
            row.prepend(f);
            table.appendChild(row);
        }
        const g = document.createElement("div");
        g.setAttribute("class", "row");
        const gId = `${this.tabId}${ROW}${this.dimension}}EL`;
        g.setAttribute("id", gId)
        for (let icell = -1; icell < this.dimension; icell++) {
            const gCell = document.createElement("div");
            const gCellId = `${gId}${icell}`;
            gCell.setAttribute("class", "gCell");
            gCell.setAttribute("id", gCellId);
            gCell.innerHTML = icell !== -1 ? `ELECTRODE ${icell}`: "";
            g.appendChild(gCell);
        }
        table.appendChild(g);
        element.appendChild(table);
    }
}