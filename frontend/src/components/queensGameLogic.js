const boardSize = 8;
const directions = [
    [1, 0], [-1, 0], [0, 1], [0, -1],
    [1, 1], [1, -1], [-1, 1], [-1, -1]
];

function generateQueens() {
    const rows = new Array(boardSize).fill(false);
    const diag1 = new Array(2 * boardSize - 1).fill(false);
    const diag2 = new Array(2 * boardSize - 1).fill(false);
    const queens = [];

    for (let col = 0; col < boardSize; col++) {
        const possibleRows = [];
        for (let row = 0; row < boardSize; row++) {
            const d1 = row - col + (boardSize - 1);
            const d2 = row + col;
            if (!rows[row] && !diag1[d1] && !diag2[d2]) {
                possibleRows.push(row);
            }
        }
        if (possibleRows.length === 0) return null;
        const row = possibleRows[Math.floor(Math.random() * possibleRows.length)];
        queens.push([row, col]);
        rows[row] = true;
        diag1[row - col + (boardSize - 1)] = true;
        diag2[row + col] = true;
    }
    return queens;
}

function inBounds(r, c) {
    return r >= 0 && r < boardSize && c >= 0 && c < boardSize;
}

function colorBoard(queensPos) {
    const board = Array.from({ length: boardSize }, () =>
        Array(boardSize).fill(null)
    );
    const frontiers = {};

    queensPos.forEach(([r, c], color) => {
        board[r][c] = {
            color,
            isQueen: true,
            marked: null,
            revealed: false
        };
        frontiers[color] = [[r, c]];
    });

    let emptyCells = boardSize * boardSize - boardSize;

    while (emptyCells > 0) {
        const colors = Object.keys(frontiers);
        if (colors.length === 0) {
            return null;
        }
        const color = Number(colors[Math.floor(Math.random() * colors.length)]);
        const frontier = frontiers[color];
        const [r, c] = frontier[Math.floor(Math.random() * frontier.length)];
        const neighbors = directions
            .slice(0, 4)
            .map(([dr, dc]) => [r + dr, c + dc])
            .filter(([nr, nc]) => inBounds(nr, nc) && board[nr][nc] === null);

        if (neighbors.length > 0) {
            const [nr, nc] = neighbors[Math.floor(Math.random() * neighbors.length)];
            board[nr][nc] = {
                color,
                isQueen: false,
                marked: null,
                revealed: false
            };
            frontier.push([nr, nc]);
            emptyCells--;
        } else {
            frontiers[color] = frontier.filter(p => p[0] !== r || p[1] !== c);
            if (frontiers[color].length === 0) delete frontiers[color];
        }
    }

    return board;
}

function hasUniqueSolution(board) {
    const regions = new Map();
    for (let r = 0; r < boardSize; r++) {
        for (let c = 0; c < boardSize; c++) {
            const color = board[r][c].color;
            if (!regions.has(color)) regions.set(color, []);
            regions.get(color).push([r, c]);
        }
    }

    const regionList = Array.from(regions.values());
    regionList.sort((a, b) => a.length - b.length);
    let solutionCount = 0;

    const rows = new Array(boardSize).fill(false);
    const cols = new Array(boardSize).fill(false);
    const mainDiag = new Array(2 * boardSize - 1).fill(false);
    const antiDiag = new Array(2 * boardSize - 1).fill(false);

    function backtrack(regionIndex) {
        if (solutionCount >= 2) return;
        if (regionIndex === regionList.length) {
            solutionCount++;
            return;
        }
        const cells = regionList[regionIndex];
        for (const [r, c] of cells) {
            const mainDiagIndex = r - c + (boardSize - 1);
            const antiDiagIndex = r + c;
            if (!rows[r] && !cols[c] && !mainDiag[mainDiagIndex] && !antiDiag[antiDiagIndex]) {
                rows[r] = true;
                cols[c] = true;
                mainDiag[mainDiagIndex] = true;
                antiDiag[antiDiagIndex] = true;
                backtrack(regionIndex + 1);
                rows[r] = false;
                cols[c] = false;
                mainDiag[mainDiagIndex] = false;
                antiDiag[antiDiagIndex] = false;
                if (solutionCount >= 2) return;
            }
        }
    }

    backtrack(0);
    return solutionCount === 1;
}

function generateNewBoard() {
    let attempts = 0;

    while (true) {
        attempts++;
        const queens = generateQueens();
        if (!queens) continue;

        const board = colorBoard(queens);
        if (!board) continue;

        if (hasUniqueSolution(board)) {
            console.log(`Unique board found after ${attempts} attempts`);
            return board;
        }

        if (attempts % 1000 === 0) {
            console.log(`Attempt ${attempts}... still searching`);
        }
    }
}

export { generateNewBoard, boardSize };