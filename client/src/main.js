import { createApp } from "vue";
import App from "./App.vue";
import { createStore } from 'vuex'


import Client from './store/connection.client'


// Create a new store instance.
const store = createStore({
    state () {
      return {
        count: 0,

      }
    },
    modules: {
      client: Client
    },
    getters: {
      getCount(state) { return state.count }
    },
    
    mutations: {
      increment (state) {
        state.count++
      }
  }
  })

  
  
  const app = createApp(App)
 
  
  // Install the store instance as a plugin
  app.use(store)
  app.mount('#app')