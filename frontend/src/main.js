import { createApp } from 'vue'
import { createRouter, createWebHistory } from 'vue-router'
import './style.css'
import App from './App.vue'
import 'vue3-chessboard/style.css';

import ChessApp from './components/ChessApp.vue'
import Connect5App from './components/Connect5App.vue'

const routes = [
  { path: '/', redirect: '/chess' },
  { path: '/chess', component: ChessApp },
  { path: '/connect5', component: Connect5App }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

createApp(App).use(router).mount('#app')