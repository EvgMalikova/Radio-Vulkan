export default {
    state: {
      favorites: []
    },
    mutations: {
      UPDATE_FAVORITES(state, payload) {
        state.favorites = payload
        console.log(state.favorites[0].name)
        
      }
    },
    actions: {
      addToFavorites(context, payload) {
          
        const favorites = context.state.favorites
        favorites.push(payload)
        context.commit('UPDATE_FAVORITES', favorites)
      }
    },
  }
  