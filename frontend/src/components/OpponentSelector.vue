<template>
  <div class="opponent-selector">
    <div
      v-for="option in options"
      :key="option.value"
      class="card"
      :class="{ selected: modelValue === option.value }"
      @click="$emit('update:modelValue', option.value)"
    >
      <img :src="option.avatar" class="avatar" />
      <div class="info">
        <div class="name">{{ option.name }}</div>
        <div class="status" :class="option.status">
          <span class="dot"></span>
          {{ option.status === 'online' ? 'Online' : 'Engine' }}
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
defineProps({
  modelValue: String,
  options: Array,
});
defineEmits(['update:modelValue']);
</script>

<style scoped>
.opponent-selector {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.card {
  display: flex;
  align-items: center;
  gap: 10px;
  border: 2px solid transparent;
  padding: 0.5rem;
  border-radius: 5px;
  cursor: pointer;
  transition: border 0.2s;
  background: #f9f9f9;
}

.card.selected {
  border-color: #007bff;
  background-color: #eaf4ff;
}

.avatar {
  width: 40px;
  height: 40px;
  border-radius: 50%;
}

.name {
  font-weight: bold;
}

.status {
  font-size: 0.8em;
  display: flex;
  align-items: center;
  color: #999;
}

.status.online {
  color: #28a745;
}

.status.engine {
  color: gray;
}

.status .dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  display: inline-block;
  margin-right: 4px;
  background-color: currentColor;
}
</style>
