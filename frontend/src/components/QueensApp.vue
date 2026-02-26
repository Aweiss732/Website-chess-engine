<template>
  <div class="app desktop" v-if="!isMobile">
    <div class="game-container">
      <div class="left-sidebar">
        <div class="game-info">
          <div class="info-box">
            <h3>Queens</h3>
            <p>Find all 8 queens on the board!</p>
            <div class="rules">
              <p>• Click to cycle: Empty → X → Queen → Empty</p>
              <p>• Queens cannot touch (even diagonally)</p>
            </div>
            <div class="quick-stats">
              <div class="stat-item">
                <span class="stat-label">Queens Found:</span>
                <span class="stat-value">{{ foundQueensCount }} / 8</span>
              </div>
              <div class="stat-item">
                <span class="stat-label">Time:</span>
                <span class="stat-value">{{ formatTime }}</span>
              </div>
              <div class="stat-item">
                <span class="stat-label">Moves:</span>
                <span class="stat-value">{{ marksCount }}</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="center-area">
        <div class="game-screen">
          <div class="game-header">
            <h1>Queens</h1>
            <div class="game-controls">
              <button @click="startNewGame" class="new-game-btn">New Game</button>
              <div class="timer-display">
                <span class="timer-label">Time:</span>
                <span class="timer-value">{{ formatTime }}</span>
              </div>
            </div>
          </div>

          <div class="board-container">
            <QueensBoard
            ref="boardRef"
            :board="board"
            :game-over="gameOver"
            :show-check-feedback="showCheckFeedback"
            @cell-mark="onCellMark"
            class="queens-board"
            />
          </div>

          <div class="game-footer">
            <div class="action-buttons">
              <button @click="checkSolution" class="check-btn">Check Solution</button>
              <button @click="showHint" class="hint-btn">Reveal Queen</button>
            </div>
          </div>
        </div>
      </div>

      <div class="right-sidebar">
        <div class="status-box">
          <div class="info-box">
            <h3>Game Status</h3>
            <div class="status-content">
              <div class="progress-bar">
                <div class="progress-fill" :style="{ width: progressPercentage + '%' }"></div>
              </div>
              <p class="progress-text">{{ progressPercentage }}% Complete</p>
              
              <div class="queen-status">
                <h4>Queens Found:</h4>
                <div class="queen-grid">
                    <div v-for="i in 8" :key="i" class="queen-status-item">
                        <div class="queen-icon" :class="{ found: queenFound(i-1) }">♕</div>
                        <div class="color-indicator" :style="{ backgroundColor: colorMap[i-1] }"></div>
                        <span class="queen-label">Queen {{ i }}</span>
                    </div>
                    </div>
              </div>
              
              <div class="hint-box" v-if="hintsUsed > 0">
                <p>Hints used: {{ hintsUsed }}</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div v-if="showGameOverPopup && gameWon" class="game-over-popup-overlay">
      <div class="game-over-popup">
        <h2>{{ gameWon ? 'Congratulations!' : 'Try Again' }}</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="stats-summary">
          <div class="stat">
            <span class="stat-label">Time:</span>
            <span class="stat-value">{{ formatTime }}</span>
          </div>
          <div class="stat">
            <span class="stat-label">Moves:</span>
            <span class="stat-value">{{ marksCount }}</span>
          </div>
          <div class="stat">
            <span class="stat-label">Hints Used:</span>
            <span class="stat-value">{{ hintsUsed }}</span>
          </div>
        </div>
        <div class="popup-buttons">
          <button @click="startNewGame" class="new-game-btn">New Game</button>
        </div>
      </div>
    </div>
  </div>

  <!-- MOBILE LAYOUT -->
  <div class="app mobile" v-else>
    <div class="mobile-container">
      <div class="mobile-game-section">
        <div class="mobile-game-screen">
          <div class="mobile-game-header">
            <h1>Queens</h1>
            <div class="mobile-game-controls">
              <button @click="startNewGame" class="new-game-btn mobile">New Game</button>
              <div class="mobile-timer">
                <span>{{ formatTime }}</span>
              </div>
            </div>
          </div>

          <div class="mobile-board-container">
            <QueensBoard
            ref="boardRef"
            :board="board"
            :game-over="gameOver"
            :show-check-feedback="showCheckFeedback"
            @cell-mark="onCellMark"
            class="mobile-queens-board"
            />
          </div>

          <div class="mobile-game-footer">
            <div class="mobile-stats">
              <div class="mobile-stat">
                <span class="stat-label">Queens:</span>
                <span class="stat-value">{{ foundQueensCount }}/8</span>
              </div>
              <div class="mobile-stat">
                <span class="stat-label">Moves:</span>
                <span class="stat-value">{{ marksCount }}</span>
              </div>
              <div class="mobile-stat">
                <span class="stat-label">Hints:</span>
                <span class="stat-value">{{ hintsUsed }}</span>
              </div>
            </div>
            <div class="mobile-actions">
              <button @click="checkSolution" class="check-btn mobile">Check</button>
              <button @click="showHint" class="hint-btn mobile">Hint</button>
            </div>
          </div>
        </div>
      </div>

      <div class="mobile-info-box">
        <div class="info-box">
          <h3>Queens</h3>
          <p>Find all 8 queens!</p>
          <div class="rules">
            <p>• Click to cycle: Empty → X → Queen → Empty</p>
            <p>• Or double-click for queen directly</p>
            <p>• Drag to mark X on multiple cells</p>
          </div>
        </div>
      </div>
    </div>

    <div v-if="showGameOverPopup && gameWon" class="game-over-popup-overlay">
      <div class="game-over-popup mobile">
        <h2>{{ gameWon ? 'Congratulations!' : 'Try Again' }}</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="popup-buttons mobile">
          <button @click="startNewGame" class="new-game-btn mobile">New Game</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import { generateNewBoard, boardSize } from './queensGameLogic.js';
import QueensBoard from './QueensBoard.vue';

const board = ref([]);
const gameOver = ref(false);
const gameWon = ref(false);
const marksCount = ref(0);
const hintsUsed = ref(0);
const startTime = ref(null);
const elapsedTime = ref(0);
const timerInterval = ref(null);
const isMobile = ref(false);
const showGameOverPopup = ref(false);
const gameOverMessage = ref('');
const boardRef = ref(null);
const showCheckFeedback = ref(false);
const lastCheckResults = ref({
  correctQueens: 0,
  wrongQueens: 0,
  missingQueens: 0
});

const colorMap = [
  '#ed8a00', // Orange
  '#593c14', // Brown
  '#205419', // Green forest
  '#8ca888', // Green olive
  '#072d70', // Blue dark
  '#81a2db', // Blue sky
  '#f590ec', // Pink
  '#a8181f'  // Red
];

const initializeGame = () => {
  board.value = generateNewBoard();
  marksCount.value = 0;
  hintsUsed.value = 0;
  gameOver.value = false;
  gameWon.value = false;
  showGameOverPopup.value = false;
  showCheckFeedback.value = false;
  lastCheckResults.value = {
    correctQueens: 0,
    wrongQueens: 0,
    missingQueens: 0
  };
  startTimer();
};

const startTimer = () => {
  if (timerInterval.value) {
    clearInterval(timerInterval.value);
  }
  startTime.value = Date.now();
  elapsedTime.value = 0;
  timerInterval.value = setInterval(() => {
    elapsedTime.value = Math.floor((Date.now() - startTime.value) / 1000);
  }, 1000);
};

const formatTime = computed(() => {
  const minutes = Math.floor(elapsedTime.value / 60);
  const seconds = elapsedTime.value % 60;
  return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
});

const foundQueensCount = computed(() => {
  if (gameOver.value && gameWon.value) {
    return 8;
  }
  return lastCheckResults.value.correctQueens;
});

const progressPercentage = computed(() => {
  return Math.round((foundQueensCount.value / 8) * 100);
});

const queenFound = (color) => {
  if (gameOver.value && gameWon.value) {
    return true;
  }
  
  let colorQueenFound = false;
  
  board.value.forEach((row) => {
    row.forEach((cell) => {
      if (cell && cell.color === color && cell.isQueen && cell.marked === 'Q') {
        colorQueenFound = true;
      }
    });
  });
  
  return colorQueenFound && lastCheckResults.value.correctQueens > 0;
};

const checkWinCondition = () => {
  if (gameOver.value) return;
  
  let correctQueens = 0;
  let wrongQueens = 0;
  
  for (let r = 0; r < boardSize; r++) {
    for (let c = 0; c < boardSize; c++) {
      const cell = board.value[r][c];
      if (cell) {
        if (cell.isQueen) {
          if (cell.marked === 'Q') {
            correctQueens++;
          }
        } else {
          if (cell.marked === 'Q') {
            wrongQueens++;
          }
        }
      }
    }
  }
  
  if (correctQueens === 8 && wrongQueens === 0) {
    gameWon.value = true;
    gameOver.value = true;
    
    lastCheckResults.value = {
      correctQueens: 8,
      wrongQueens: 0,
      missingQueens: 0
    };
    
    gameOverMessage.value = 'Perfect! You found all 8 queens correctly!';
    showGameOverPopup.value = true;
    
    if (timerInterval.value) {
      clearInterval(timerInterval.value);
    }
  }
};

const onCellMark = (row, col) => {
  if (gameOver.value) return;
  
  const cell = board.value[row][col];
  if (!cell) return;
  
  showCheckFeedback.value = false;
  
  if (cell.marked === null) {
    cell.marked = 'X';
  } else if (cell.marked === 'X') {
    cell.marked = 'Q';
  } else if (cell.marked === 'Q') {
    cell.marked = null;
  }
  marksCount.value++;
  
  board.value = [...board.value];
  
  checkWinCondition();
};

const checkSolution = () => {
  if (gameOver.value) return;
  
  let correctQueens = 0;
  let wrongQueens = 0;
  let missingQueens = 0;
  
  for (let r = 0; r < boardSize; r++) {
    for (let c = 0; c < boardSize; c++) {
      const cell = board.value[r][c];
      if (cell) {
        if (cell.isQueen) {
          if (cell.marked === 'Q') {
            correctQueens++;
          } else {
            missingQueens++;
          }
        } else {
          if (cell.marked === 'Q') {
            wrongQueens++;
          }
        }
      }
    }
  }
  
  lastCheckResults.value = {
    correctQueens,
    wrongQueens,
    missingQueens
  };
  
  showCheckFeedback.value = true;
  
  setTimeout(() => {
    showCheckFeedback.value = false;
  }, 2000);
  
  checkWinCondition();
};

const showHint = () => {
  if (gameOver.value) return;
  
  const unrevealedQueens = [];
  board.value.forEach((row, rowIndex) => {
    row.forEach((cell, colIndex) => {
      if (cell && cell.isQueen && cell.marked !== 'Q') {
        unrevealedQueens.push({ row: rowIndex, col: colIndex });
      }
    });
  });
  
  if (unrevealedQueens.length > 0) {
    const randomQueen = unrevealedQueens[Math.floor(Math.random() * unrevealedQueens.length)];
    board.value[randomQueen.row][randomQueen.col].marked = 'Q';
    hintsUsed.value++;
    board.value = [...board.value];
  }
};

const startNewGame = () => {
  if (timerInterval.value) {
    clearInterval(timerInterval.value);
  }
  initializeGame();
};

onMounted(() => {
  const checkMobile = () => {
    isMobile.value = window.innerWidth <= 768;
  };
  
  checkMobile();
  window.addEventListener('resize', checkMobile);
  
  initializeGame();
  
  onUnmounted(() => {
    window.removeEventListener('resize', checkMobile);
    if (timerInterval.value) {
      clearInterval(timerInterval.value);
    }
  });
});
</script>

<style scoped>
@font-face {
  font-family: 'BurtonsNightmare';
  src: url('../assets/fonts/NITEMARE.TTF') format('truetype');
  font-weight: normal;
  font-style: normal;
  font-display: swap;
}

.app.desktop {
  min-height: calc(100vh - 80px);
  padding: 1rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.game-container {
  display: flex;
  width: 100%;
  max-width: 1200px;
  margin: 0 auto;
  gap: 2rem;
  align-items: flex-start;
  justify-content: center;
  flex-wrap: nowrap;
}

.left-sidebar,
.right-sidebar {
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 36px;
}

.left-sidebar {
  flex: 0 0 200px;
}

.right-sidebar {
  flex: 0 0 350px;
  padding-left: 40px;
}

.center-area {
  width: 500px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
  padding-top: 4px;
  box-sizing: border-box;
  margin-left: 85px;
}

.game-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.game-header {
  text-align: center;
  margin-bottom: 0.25rem;
}

.game-header h1 {
  font-family: 'BurtonsNightmare', cursive;
  font-size: 2.5rem;
  color: #556B2F;
  margin-bottom: 0.25rem;
  text-shadow: 2px 2px 4px rgba(139, 69, 19, 0.2);
}

.game-controls {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.5rem 1rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 8px;
  border: 2px solid #8FBC8F;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
  margin-bottom: 0.25rem;
  gap: 1rem;
}

.new-game-btn {
  background: linear-gradient(135deg, #8FBC8F 0%, #556B2F 100%);
  color: white;
  border: none;
  padding: 0.5rem 1rem;
  font-size: 1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(85, 107, 47, 0.3);
}

.new-game-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(85, 107, 47, 0.4);
  background: linear-gradient(135deg, #9fcc9f 0%, #657b3f 100%);
}

.timer-display {
  font-size: 1.2rem;
  font-weight: 600;
  color: #556B2F;
}

.timer-label {
  margin-right: 0.5rem;
}

.board-container {
  display: flex;
  align-items: flex-start;
  justify-content: center;
  gap: 1px;
  margin: 0;
  width: 100%;
  box-sizing: border-box;
}

.queens-board {
  width: 400px;
  height: 400px;
  margin: 0 auto;
}

.game-footer {
  margin-top: 1rem;
  text-align: center;
  width: 100%;
}

.action-buttons {
  display: flex;
  justify-content: center;
  gap: 1rem;
}

.check-btn, .hint-btn {
  padding: 0.5rem 1.5rem;
  border: none;
  border-radius: 6px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

.check-btn {
  background: linear-gradient(135deg, #c4dc91 0%, #7b9740 100%);
  color: white;
}

.hint-btn {
  background: linear-gradient(135deg, #a2917f 0%, #4a3927 100%);
  color: white;
}

.check-btn:hover, .hint-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
}

.status-box {
  width: 100%;
}

.info-box {
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  padding: 1.5rem;
  border-radius: 8px;
  border: 2px solid #8FBC8F;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
}

.info-box h3 {
  color: #556B2F;
  margin-bottom: 0.5rem;
  font-family: 'BurtonsNightmare', cursive;
  text-align: center;
  font-size: 1.5rem;
}

.info-box p {
  color: #5a4a3a;
  margin: 0.5rem 0;
  text-align: center;
}

.rules {
  margin-top: 1rem;
  font-size: 0.9rem;
}

.rules p {
  text-align: left;
  margin: 0.25rem 0;
  font-size: 0.8rem;
}

.quick-stats {
  margin-top: 1.5rem;
  padding-top: 1rem;
  border-top: 1px solid #e2d6c9;
}

.stat-item {
  display: flex;
  justify-content: space-between;
  margin-bottom: 0.5rem;
  font-size: 0.9rem;
}

.stat-label {
  color: #5a4a3a;
  font-weight: 500;
}

.stat-value {
  color: #556B2F;
  font-weight: 600;
  font-family: 'BurtonsNightmare', cursive;
}

.progress-bar {
  height: 20px;
  background: #e0e0e0;
  border-radius: 10px;
  overflow: hidden;
  margin-bottom: 0.5rem;
}

.progress-fill {
  height: 100%;
  background: linear-gradient(90deg, #8FBC8F 0%, #556B2F 100%);
  transition: width 0.3s ease;
}

.progress-text {
  text-align: center;
  margin-bottom: 1rem;
  font-weight: 600;
  color: #556B2F;
}

.queen-status {
  margin-top: 1rem;
}

.queen-status h4 {
  margin-bottom: 0.5rem;
  color: #556B2F;
  text-align: center;
}

.queen-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 0.5rem;
}

.queen-status-item {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.25rem;
  background: #f8f6f4;
  border-radius: 4px;
}
.color-indicator {
  width: 16px;
  height: 16px;
  border-radius: 2px;
  border: 1px solid rgba(0,0,0,0.2);
  flex-shrink: 0;
}

.queen-icon {
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #f0f0f0;
  border-radius: 4px;
  font-size: 1.2rem;
  color: #999;
}

.queen-icon.found {
  background: #8FBC8F;
  color: white;
}

.queen-label {
  font-size: 0.9rem;
  color: #5a4a3a;
  flex-grow: 1;
}

.queen-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 0.5rem;
}

.hint-box {
  margin-top: 1rem;
  padding-top: 1rem;
  border-top: 1px solid #e2d6c9;
  text-align: center;
  font-size: 0.9rem;
  color: #5a4a3a;
}

.stats-summary {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1rem;
  margin: 1.5rem 0;
}

.stat {
  text-align: center;
  padding: 0.5rem;
  background: #f8f6f4;
  border-radius: 6px;
}

.stat-label {
  display: block;
  font-size: 0.9rem;
  color: #5a4a3a;
}

.stat-value {
  display: block;
  font-size: 1.2rem;
  font-weight: 600;
  color: #556B2F;
  font-family: 'BurtonsNightmare', cursive;
}

.popup-buttons {
  display: flex;
  justify-content: center;
  margin-top: 1.5rem;
}

.game-over-popup-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.7);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.game-over-popup {
  background: linear-gradient(135deg, #FFF8DC 0%, #F5F5DC 100%);
  padding: 2rem;
  border-radius: 12px;
  border: 3px solid #8FBC8F;
  text-align: center;
  max-width: 350px;
  width: 90%;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.game-over-popup h2 {
  color: #556B2F;
  margin-bottom: 1rem;
  font-size: 1.5rem;
}

.game-over-popup p {
  color: #5a4a3a;
  margin-bottom: 1.5rem;
  font-size: 1.1rem;
}

/* MOBILE STYLES */
.app.mobile {
  min-height: 100vh;
  padding: 0.5rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  overflow-x: hidden;
}

.mobile-container {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  max-width: 100%;
  margin: 0 auto;
  min-height: 100vh;
  width: 100%;
}

.mobile-game-section {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.mobile-game-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.mobile-game-header {
  text-align: center;
  width: 100%;
}

.mobile-game-header h1 {
  font-family: 'BurtonsNightmare', cursive;
  font-size: 2rem;
  color: #556B2F;
  margin-bottom: 0.5rem;
}

.mobile-game-controls {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.5rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 6px;
  border: 2px solid #8FBC8F;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
  margin-bottom: 0.5rem;
}

.mobile-board-container {
  display: flex;
  justify-content: center;
  margin: 0.25rem 0;
  width: 100%;
  max-width: 100vw;
  box-sizing: border-box;
  padding: 0 0.5rem;
}

.mobile-queens-board {
  width: 300px;
  height: 300px;
}

.mobile-game-footer {
  padding: 0.5rem;
  width: 100%;
}

.mobile-stats {
  display: flex;
  justify-content: space-around;
  margin-bottom: 0.5rem;
  padding: 0.5rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 6px;
  border: 1px solid #e2d6c9;
}

.mobile-stat {
  text-align: center;
}

.mobile-stat .stat-label {
  display: block;
  font-size: 0.8rem;
  color: #5a4a3a;
}

.mobile-stat .stat-value {
  display: block;
  font-size: 1rem;
  font-weight: 600;
  color: #556B2F;
  font-family: 'BurtonsNightmare', cursive;
}

.mobile-actions {
  display: flex;
  justify-content: center;
  gap: 0.5rem;
}

.mobile-actions button {
  padding: 0.5rem 1rem;
  font-size: 0.9rem;
}

.mobile-info-box {
  width: 100%;
  margin-top: 0.5rem;
}

.mobile-info-box .info-box {
  padding: 1rem;
  font-size: 0.9rem;
}

.mobile-info-box .info-box h3 {
  font-size: 1.3rem;
}

.mobile-info-box .rules p {
  font-size: 0.8rem;
  margin: 0.2rem 0;
}

.mobile-info-box .stat-item {
  font-size: 0.8rem;
}

/* Mobile Game Over Popup */
@media (max-width: 768px) {
  .game-over-popup {
    padding: 1.5rem;
    max-width: 280px;
  }
  
  .game-over-popup h2 {
    font-size: 1.3rem;
  }
  
  .game-over-popup p {
    font-size: 1rem;
    margin-bottom: 1.25rem;
  }
  
  .popup-buttons {
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
  }
  
  .popup-buttons button {
    width: 80%;
    margin: 0;
  }

  .color-indicator {
    width: 14px;
    height: 14px;
  }
  
  .queen-icon {
    width: 20px;
    height: 20px;
    font-size: 1rem;
  }
  
  .queen-label {
    font-size: 0.8rem;
  }
}

@media (max-width: 360px) {
  .app.mobile {
    padding: 0.25rem;
  }
  
  .mobile-container {
    gap: 0.4rem;
  }
  
  .mobile-game-header h1 {
    font-size: 1.7rem;
  }
  
  .mobile-queens-board {
    width: 280px;
    height: 280px;
  }
  
  .mobile-actions button {
    padding: 0.4rem 0.8rem;
    font-size: 0.8rem;
  }
}

@media (min-width: 769px) {
  .app.mobile {
    display: none;
  }
  
  .app.desktop {
    display: flex;
  }
}

@media (max-width: 768px) {
  .app.desktop {
    display: none;
  }
  
  .app.mobile {
    display: block;
  }
}
</style>