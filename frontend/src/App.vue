<template>
  <div id="app">
    <AppNavigation />
    <router-view />
  </div>
</template>

<script setup>
import AppNavigation from './components/AppNavigation.vue'
</script>

<style>
#app {
  min-height: 100vh;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
}
</style>