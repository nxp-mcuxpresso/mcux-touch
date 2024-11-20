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

/* NT cross talk class */
class CrossTalk extends Matrix {
    constructor(dimension, tabId, touchTimes) {
        super(dimension, tabId);
        this.accuCount = {
            "prev": 0,
            "next": 0
        }
        this.varMap = {
            "actMat[0]": {
                "tabId": `${tabId}${ROW}0${CELL}0`,
                "value": "",
                "changed": true,
                "position": { "row": 0, "cell": 0 },
                "pressedCount": touchTimes,
            },
            "actMat[1]": {
                "tabId": `${tabId}${ROW}0${CELL}1`,
                "value": "",
                "changed": false,
                "position": { "row": 0, "cell": 1 },
                "pressedCount": 0,
            },
            "actMat[2]": {
                "tabId": `${tabId}${ROW}0${CELL}2`,
                "value": "",
                "changed": false,
                "position": { "row": 0, "cell": 2 },
                "pressedCount": 0,
            },
            "actMat[3]": {
                "tabId": `${tabId}${ROW}0${CELL}3`,
                "value": "",
                "changed": false,
                "position": { "row": 0, "cell": 3 },
                "pressedCount": 0,
            },

            "actMat[4]": {
                "tabId": `${tabId}${ROW}1${CELL}0`,
                "value": "",
                "changed": false,
                "position": { "row": 1, "cell": 0 },
                "pressedCount": 0,
            },

            "actMat[5]": {
                "tabId": `${tabId}${ROW}1${CELL}1`,
                "value": "",
                "changed": true,
                "position": { "row": 1, "cell": 1 },
                "pressedCount": touchTimes,
            },
            "actMat[6]": {
                "tabId": `${tabId}${ROW}1${CELL}2`,
                "value": "",
                "changed": false,
                "position": { "row": 1, "cell": 2 },
                "pressedCount": 0,
            },
            "actMat[7]": {
                "tabId": `${tabId}${ROW}1${CELL}3`,
                "value": "",
                "changed": false,
                "position": { "row": 1, "cell": 3 },
                "pressedCount": 0,
            },

            "actMat[8]": {
                "tabId": `${tabId}${ROW}2${CELL}0`,
                "value": "",
                "changed": false,
                "position": { "row": 2, "cell": 0 },
                "pressedCount": 0,
            },
            "actMat[9]": {
                "tabId": `${tabId}${ROW}2${CELL}1`,
                "value": "",
                "changed": false,
                "position": { "row": 2, "cell": 1 },
                "pressedCount": 0,
            },
            "actMat[10]": {
                "tabId": `${tabId}${ROW}2${CELL}2`,
                "value": "",
                "changed": true,
                "position": { "row": 2, "cell": 2 },
                "pressedCount": touchTimes,
            },
            "actMat[11]": {
                "tabId": `${tabId}${ROW}2${CELL}3`,
                "value": "",
                "changed": false,
                "position": { "row": 2, "cell": 3 },
                "pressedCount": 0,
            },

            "actMat[12]": {
                "tabId": `${tabId}${ROW}3${CELL}0`,
                "value": "",
                "changed": false,
                "position": { "row": 3, "cell": 0 },
                "pressedCount": 0,
            },
            "actMat[13]": {
                "tabId": `${tabId}${ROW}3${CELL}1`,
                "value": "",
                "changed": false,
                "position": { "row": 3, "cell": 1 },
                "pressedCount": 0,
            },
            "actMat[14]": {
                "tabId": `${tabId}${ROW}3${CELL}2`,
                "value": "",
                "changed": false,
                "position": { "row": 3, "cell": 2 },
                "pressedCount": 0,
            },
            "actMat[15]": {
                "tabId": `${tabId}${ROW}3${CELL}3`,
                "value": "",
                "changed": true,
                "position": { "row": 3, "cell": 3 },
                "pressedCount": touchTimes,
            },
        }
        this.touchTimes = typeof touchTimes === "undefined" ? -1 : touchTimes;
        this.average = new Array(dimension).fill(0);
        this.denominator = 0;
        this.line = 0;
    }

    getFontColor(value) {
        return `hsl(${Math.round(value / this.touchTimes * 240)}, 50%, 75%)`;
    }

    createMatrix(element) {
        const touchName = document.createElement("h2");
        touchName.innerHTML = "Current Touch matrix";
        touchName.style.textAlign = "center";
        touchName.style.color = "#0A87EF";
        touchName.style.textTransform = "uppercase";
        touchName.style.fontFamily = "Lato";
        const littlHeader = document.createElement("h4");
        littlHeader.innerHTML = "Touches remaining";
        littlHeader.style.float = "right";
        littlHeader.style.marginRight = "5%";
        element.appendChild(touchName);
        element.appendChild(littlHeader);
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
            if (typeof this.touchTimes !== -1) {
                const touches = document.createElement("div");
                touches.setAttribute("class", "cell");
                touches.style.borderColor = "white";

                const check = document.createElement("img");
                check.setAttribute("id", `${rowId}check`);
                check.setAttribute("src", "img/detected.svg");
                check.setAttribute("class", "check");

                const touchT = document.createElement("p");
                touchT.style.fontWeight = "bold";
                touchT.style.float = "left";
                touchT.style.margin = "1vh";
                touchT.style.color = this.getFontColor(this.touchTimes);
                touchT.setAttribute("id", `${rowId}${TOUCH}`);
                touchT.innerHTML = this.touchTimes;
                touches.appendChild(touchT);
                touches.appendChild(check);
                row.appendChild(touches);
            }
            const f = document.createElement("div");
            f.setAttribute("class", "cell");
            f.setAttribute("id", `${rowId}EL${irow}`);
            f.innerHTML = `ELECTRODE ${irow}`;
            f.style.color = "#00A6FF";
            f.style.borderColor = "white";
            f.style.fontSize = "11px";
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
            gCell.innerHTML = icell !== -1 ? `ELECTRODE ${icell}` : "";
            g.appendChild(gCell);
        }
        table.appendChild(g);

        element.appendChild(table);
        element.appendChild(document.createElement("br"));
        const row = document.createElement("div");
        const status = document.createElement("p");
        const mark = document.createElement("p");
        mark.setAttribute("id", "status");
        mark.innerHTML = "incomplete"
        mark.style.color = "red";
        status.innerHTML = "Calibration status:";
        status.append(mark);
        row.style.textAlign = "center";
        row.style.justifyContent = "center";
        row.appendChild(status);
        element.appendChild(row);
    }

    fillValues() {
        const zip = (...arr) => {
            const zipped = [];
            arr.forEach((element, ind) => {
               element.forEach((el, index) => {
                  if(!zipped[index]){
                     zipped[index] = [];
                  };
                  if(!zipped[index][ind]){
                     zipped[index][ind] = [];
                  }
                  zipped[index][ind] = el || '';
               })
            });
            return zipped;
         };
        const sum = (l) => l.reduce((a, b) => a + b, 0);

        this.readValues("values.txt", (data) => {
            if (data !== null) {
                var list = data.trim().split("\n");
                list = list.map(i => JSON.parse(i));
                this.line = list.length;
                this.denominator = this.touchTimes * this.line;
                for (let i = 0; i < this.dimension; i++) {
                    const tmp = [];
                    list.forEach(lst => tmp.push(lst[i]));
                    var av = zip(...tmp);
                    this.average[i] = av.map(i => sum(i));
                }
            } else {
                this.average = Array.from(Array(this.dimension), () => new Array(this.dimension).fill(0));
            }
        });
    }

    calculateAverageOfAverages() {
        var contents = "int16_t actMat[NT_XTALK_NSENSORS * NT_XTALK_NSENSORS] = {\n";
        this.average.forEach(val => {
            const avg = this.denominator !== 0 ? val.map(i => Math.round(i / this.denominator)) : val;
            contents += `\t${avg.join(", ")}, \n`;
        });
        contents += "};\n";
        return contents;
    }

    dump() {
        const diagIds = ["actMat[0]", "actMat[5]", "actMat[10]", "actMat[15]"];
        const contents = JSON.stringify(this.values);
        this.line += 1;
        this.denominator = this.line * this.touchTimes;
        this.values.forEach((i, index) => {
            i.forEach((j, index1) => {
                this.average[index][index1] += j;
            });
        });
        Object.keys(this.varMap).forEach(key => {
            if (!diagIds.includes(key)) {
                this.varMap[key].changed = false;
                this.varMap[key].pressedCount = 0;
                this.values[this.varMap[key].position.row][this.varMap[key].position.cell] = 0;
            }
            const touchId = `${this.tabId}${ROW}${this.varMap[key].position.row}${TOUCH}`;
            const touchElement = document.getElementById(touchId);
            const checkId = `${this.tabId}${ROW}${this.varMap[key].position.row}check`;
            const checkElement = document.getElementById(checkId);
            checkElement.style.display = "none";
            touchElement.innerHTML = this.touchTimes;
        });
        this.readValues("values.txt", (data) => {
            console.log(data);
            data = data === null ? `${contents}\n` : `${data}${contents}\n`;
            this.saveValues(data, "values.txt");
        });

    }

    calculateAverage(mapField) {
        const avgId = `tabavg${ROW}${mapField.position.row}${CELL}${mapField.position.cell}`;
        const sum = this.values[mapField.position.row][mapField.position.cell];
        const avg = Math.round(sum / mapField.pressedCount);
        this.setValue(avg, avgId);
    }

    variables() {
        pcm.OnVariableChanged = (name, id, value) => {
            if (name === "profile_accu_count") {
                const curr = this.accuCount.next;
                this.accuCount.next = value;
                this.accuCount.prev = curr;
            }
            if (name.startsWith("actMat")) {
                const mapField = this.varMap[name];
                const cellId = mapField.tabId;
                mapField.value = value;
                if (this.accuCount.next === 0 && this.accuCount.prev !== 0) {
                    const touchId = `${this.tabId}${ROW}${mapField.position.row}${TOUCH}`;
                    const touchElement = document.getElementById(touchId);
                    mapField.pressedCount += 1;
                    if (mapField.pressedCount <= this.touchTimes) {
                        const delta = this.touchTimes - mapField.pressedCount;
                        touchElement.innerHTML = delta;
                        touchElement.style.color = this.getFontColor(delta);
                        if (delta === 0) {
                            const checkId = `${this.tabId}${ROW}${mapField.position.row}check`;
                            const checkElement = document.getElementById(checkId);
                            checkElement.style.display = "block";

                        }
                    }
                    this.values[mapField.position.row][mapField.position.cell] += value;
                    this.calculateAverage(mapField, value);
                    mapField.changed = true;
                    const allPressed = Object.keys(this.varMap).every(e => this.varMap[e].changed && this.varMap[e].pressedCount >= this.touchTimes);
                    if (allPressed) {
                        console.log("dumping");
                        const status = document.getElementById("status");
                        status.innerHTML = "complete";
                        status.style.color = "green";
                        this.dump();
                    }
                }
                this.setValue(value, cellId);
            }
        };
        for (let i = 0; i < 16; i++) {
            const varName = `actMat[${i}]`;
            pcm.SubscribeVariable(varName).then(() => { debug_print(`${varName} was subscribed`) }).catch(err => debug_print(err));
        }
        pcm.SubscribeVariable("profile_accu_count").then(() => { }).catch(err => debug_print(err));

    }
}