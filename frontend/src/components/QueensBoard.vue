<template>
  <div class="queens-board">
    <div class="board-grid">
      <div v-for="(row, rowIndex) in board" :key="rowIndex" class="board-row">
        <div v-for="(cell, colIndex) in row" :key="colIndex" 
             :class="getCellClasses(rowIndex, colIndex)"
             @click="handleClick(rowIndex, colIndex)">
          <div class="cell-content">
            <span v-if="cell && cell.marked === 'Q'" class="queen-icon">♕</span>
            <span v-else-if="cell && cell.marked === 'X'" class="x-mark">✕</span>
            <div v-if="cell" 
                 class="color-fill" 
                 :style="{ backgroundColor: getColor(cell.color) }">
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue';

const props = defineProps({
  board: {
    type: Array,
    required: true
  },
  gameOver: {
    type: Boolean,
    default: false
  },
  showCheckFeedback: {
    type: Boolean,
    default: false
  }
});

const emit = defineEmits(['cell-mark']);

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

const getColor = (colorIndex) => {
  return colorMap[colorIndex] || '#CCCCCC';
};

const getCellClasses = (rowIndex, colIndex) => {
  const cell = props.board[rowIndex][colIndex];
  const classes = ['board-cell'];
  
  if (cell) {
    if (props.showCheckFeedback) {
      if (cell.marked === 'Q') {
        if (cell.isQueen) {
          classes.push('correct-queen');
        } else {
          classes.push('wrong-queen');
        }
      } else if (cell.marked === 'X') {
        if (cell.isQueen) {
          classes.push('wrong-x');
        }
      }
    } else {
      if (cell.marked === 'Q') {
        classes.push('queen-marked');
      } else if (cell.marked === 'X') {
        classes.push('x-marked');
      }
    }
    

    classes.push('cell-border');
  }
  
  return classes.join(' ');
};

const handleClick = (row, col) => {
  if (props.gameOver) return;
  emit('cell-mark', row, col);
};
</script>

<style scoped>
.queens-board {
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  background: #DEB887;
  border: 3px solid #725334;
  border-radius: 4px;
  padding: 10px;
  box-shadow: 0 4px 12px rgba(139, 69, 19, 0.3);
  user-select: none;
  -webkit-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
}

.board-grid {
  display: grid;
  grid-template-columns: repeat(8, 1fr);
  grid-template-rows: repeat(8, 1fr);
  gap: 1px;
  width: 100%;
  height: 100%;
}

.board-row {
  display: contents;
}

.board-cell {
  aspect-ratio: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  position: relative;
  transition: all 0.1s ease;
  background: #c19a6b;
  touch-action: none;
}

.board-cell:hover {
  background: rgba(255, 255, 255, 0.1);
}

.board-cell.cell-border {
  border: 1px solid rgba(139, 69, 19, 0.3);
}

.color-fill {
  width: 100%;
  height: 100%;
  position: absolute;
  top: 0;
  left: 0;
  opacity: 0.7;
  transition: opacity 0.2s ease;
}

.queen-marked .color-fill,
.x-marked .color-fill {
  opacity: 0.7;
}

.queen-marked {
}

.x-marked {
}

.correct-queen {
  background: linear-gradient(135deg, #54b157 0%, #2E7D32 100%) !important;
  box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.3);
  border: 2px solid #1B5E20 !important;
  transition: all 0.3s ease;
}

.wrong-queen {
  background: linear-gradient(135deg, #ff1100 0%, #a30000 100%) !important;
  box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.3);
  border: 2px solid #B71C1C !important;
  transition: all 0.3s ease;
}

.wrong-x {
  background: linear-gradient(135deg, #FF9800 0%, #EF6C00 100%) !important;
  box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.2);
  border: 2px solid #B71C1C !important;
  transition: all 0.3s ease;
}

.cell-content {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
}

.queen-icon {
  font-size: 1.8rem;
  color: #FFFFFF;
  text-shadow: 
    0px 0px 3px rgba(0, 0, 0, 0.8),
    1px 1px 2px rgba(0, 0, 0, 0.5);
  z-index: 2;
  position: relative;
  font-weight: bold;
}

.x-mark {
  font-size: 2rem;
  color: #FFFFFF;
  font-weight: bold;
  text-shadow: 
    0px 0px 3px rgba(0, 0, 0, 0.8),
    1px 1px 2px rgba(0, 0, 0, 0.5);
  z-index: 2;
  position: relative;
}

.x-marked .x-mark {
  color: #dedede;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
}

.queen-marked .queen-icon {
  color: #dedede;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
}

.correct-queen .queen-icon {
  color: white !important;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.7) !important;
  font-size: 2rem !important;
  font-weight: bold !important;
}

.wrong-queen .queen-icon {
  color: white !important;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.7) !important;
  font-size: 2rem !important;
  font-weight: bold !important;
}

.wrong-x .x-mark {
  color: white !important;
  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.7) !important;
  font-size: 2.2rem !important;
  font-weight: bold !important;
}

/* Mobile styles */
@media (max-width: 768px) {
  .queen-icon {
    font-size: 1.5rem;
  }
  
  .x-mark {
    font-size: 1.8rem;
  }
  
  .correct-queen .queen-icon,
  .wrong-queen .queen-icon {
    font-size: 1.7rem !important;
  }
  
  .wrong-x .x-mark {
    font-size: 2rem !important;
  }
}

.board-cell:active {
  -webkit-touch-callout: none;
  -webkit-user-select: none;
  -khtml-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
  user-select: none;
}
</style>