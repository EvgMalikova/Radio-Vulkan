import {serializeKeyUp,serializeKeyDown,serializeMouseUp,serializeMouseDown,serializeMouseDrag} from "../serializers";
import {mousePosition} from "../mouse";
import {appConfig} from "../config";
export default {
    namespaced: true,
    state: {
      client: null,
      config: null,
      connected: false,
      connection: null,
      wsImage: "ws://localhost:8881",
      wsCommand: "ws://localhost:8882",
      image:null,
      src:"https://github.com/EvgMalikova/Radio-Vulkan/blob/first/WSRTI/webappclient/appclient/placeholder.jpg?raw=true"
      
    },
    data: () => ({ 
      FRAMES :0,
      ELAPSED :0,
      FPS :0,
      IMAGE_SIZE:0,
      eventBuffer:null
    }),
    
    mutations: {
      UPDATE_CONFIG(state, payload) {
        state.config = payload
        //console.log(state.config.connection)
        
      },
      UPDATE_IMAGE(state, payload) {
        state.src = payload
        //console.log(state.config.connection)
        
      },
      SET_CONNECTED(state, payload) {
        state.connected = payload      
      }
    },
    actions: {
      getAppConnection(context)
      { 
        console.log(appConfig)
        context.commit('UPDATE_CONFIG', appConfig)

      },

      SET_IMAGE(state, img)
      {  //if(state.connected){
        this.image=img;
        console.log('image got')
        console.log(this.image)
      //}
         

      },
      MOUSE_DOWN(state, evt)
      {
        //if(state.connected)
        function modifiers(evt) {
          let m = 0;
          if(evt.altKey) m |= 0x1;
          if(evt.ctrlKey) m |= 0x2;
          if(evt.metaKey) m |= 0x4;
          if(evt.shiftKey) m |= 0x8;
          return m;
        }
        evt.preventDefault();
        
        const p = mousePosition(evt.clientX, evt.clientY, this.image);
        //this.onMouseDown(this.image, p, evt.buttons, modifiers(evt));
        this.image.focus();
        console.log("MouseDown")
        serializeMouseDown(this.eventBuffer, p, evt.buttons, modifiers(evt));
        this.connection.send(this.eventBuffer);
       
       //console.log(evt)

      },

      MOUSE_UP(state, evt) {
        function modifiers(evt) {
          let m = 0;
          if(evt.altKey) m |= 0x1;
          if(evt.ctrlKey) m |= 0x2;
          if(evt.metaKey) m |= 0x4;
          if(evt.shiftKey) m |= 0x8;
          return m;
        }
        evt.preventDefault();
        
        const p = mousePosition(evt.clientX, evt.clientY, this.image);
        //this.onMouseDown(this.image, p, evt.buttons, modifiers(evt));
        this.image.focus();
        serializeMouseUp(this.eventBuffer, p, evt.buttons, modifiers(evt));
        this.connection.send(this.eventBuffer);
      },
      
      MOUSE_MOVE(state, evt) {
        function modifiers(evt) {
          let m = 0;
          if(evt.altKey) m |= 0x1;
          if(evt.ctrlKey) m |= 0x2;
          if(evt.metaKey) m |= 0x4;
          if(evt.shiftKey) m |= 0x8;
          return m;
        }
        evt.preventDefault();
        
        const p = mousePosition(evt.clientX, evt.clientY, this.image);
        //this.onMouseDown(this.image, p, evt.buttons, modifiers(evt));
        this.image.focus();
        if(evt.buttons) {
          serializeMouseDrag(this.eventBuffer, p, evt.buttons, modifiers(evt));
          this.connection.send(this.eventBuffer);
        }
      },
      KEY_DOWN(state, evt) {
        function buildKeyEventData(evt) {
          let k = evt.key.length == 1 ?
                  evt.key.charCodeAt(0) : evt.keyCode;
          let r = false;
          let s = false;
          if(evt.key.length > 1) {
            s = true;
            if(evt.code.endsWith("Right")) {
              r = true;
            }
          }return {key: k,
            alt: evt.altKey,
            ctrl: evt.ctrlKey,
            meta: evt.metaKey,
            shift: evt.shiftKey,
            code: evt.code,
            special: s,
            right: r};
      }
   
        serializeKeyDown(this.eventBuffer, buildKeyEventData(evt));
        this.connection.send(this.eventBuffer);
      },
      KEY_UP(state, evt) {
        function buildKeyEventData(evt) {
          let k = evt.key.length == 1 ?
                  evt.key.charCodeAt(0) : evt.keyCode;
          let r = false;
          let s = false;
          if(evt.key.length > 1) {
            s = true;
            if(evt.code.endsWith("Right")) {
              r = true;
            }
          }return {key: k,
            alt: evt.altKey,
            ctrl: evt.ctrlKey,
            meta: evt.metaKey,
            shift: evt.shiftKey,
            code: evt.code,
            special: s,
            right: r};
      }
        serializeKeyUp(this.eventBuffer, buildKeyEventData(evt));
        this.connection.send(this.eventBuffer);
      },
      
     

    
      
      //CONNECT({ state, commit, dispatch }) {
      connect(context)
      {
        const conn= context.state.connected;
        console.log(conn);
        console.log( this.image)
          if(!conn)
          {
            //saving config to store
          //const conf=appConfig;
          
          
          console.log("Starting connection to WebSocket Server")
          //console.log(serverCommandIDs.nullcommand)
          this.wsImage="ws://"+context.state.config.connection.wsHost+":"+ context.state.config.connection.wsImageStreamPort;
          //console.log(this.wsImage);
          this.connection = new WebSocket(this.wsImage);//("ws://localhost:8881")

          var self = this;
          this.connection.onmessage = function (e) {
            

            let urlCreator = window.URL || window.webkitURL;
            let imageUrl = ['placeholder.jpg', 'placeholder.jpg'];
            let front = 1, back = 0;
            urlCreator.revokeObjectURL(imageUrl[back]);
            imageUrl[back] = urlCreator.createObjectURL(e.data);
            [front, back] = [back, front];
            
            self.src = imageUrl[front];
            context.commit('UPDATE_IMAGE', imageUrl[front])
            //console.log(self.src)
          }

          this.connection.binaryType = "blob";
          this.connection.onopen = function(event) {
            console.log(event)
            console.log("Successfully connected to the echo websocket server...")
            context.commit('SET_CONNECTED', true)

      }
         
          this.eventBuffer = new Int32Array(4);
      

        }
          else{
            //disconnect
          }
         
          
          
      },
      
    },
    getters: {
      
      CONFIG(state) {
          return state.config;
        },
      isConnected: function (state) {
        //console.log(`${state.connected}`)
        //return `${state.connected}`;//
       // const { client } = state;
        //if (state.connect) return "Disconnect"
        //else return "Connect";
        return state.connected;
      },
      getImageUrl: function (state) {
        return `${state.src}`
      }
    },
    methods:
    { 

      
      /*onMouseDown(element, p, buttons, modifiers, ) {
        element.focus();
        console.log("MouseDown")
        serializeMouseDown(this.eventBuffer, p, buttons, modifiers);
        this.connection.send(this.eventBuffer);
      },*/
      

      

      /** Configure websocket to receive blobs and display the received data
    through an img html element */
  setupWSImageStreamHandler(wsocket, image, benchmark) {
  wsocket.binaryType = "blob";
  let urlCreator = window.URL || window.webkitURL;
  let imageUrl = ['placeholder.jpg', 'placeholder.jpg'];
  let front = 1, back = 0;
  if(benchmark) {
    let prev = performance.now();
    this.FRAMES = 0;
    this.ELAPSED = 0;
    this.FPS = 0;
    this.IMAGE_SIZE = 0;
    wsocket.onmessage = function (e) {
      console.log("Message received")
      const cur = performance.now();
      this.ELAPSED += cur - prev;
      this.IMAGE_SIZE = e.data.size;
      if(this.FRAMES && this.FRAMES % 50 === 0) {
        this.FPS = (1000 * 50) / this.ELAPSED;
        this.ELAPSED /= 50;
        this.FRAMES = 1;
      }
      this.FRAMES++;
      prev = cur;
      //if(FPS_GUI) FPS_GUI.html(FPS.toFixed());
      urlCreator.revokeObjectURL(imageUrl[back]);
      imageUrl[back] = urlCreator.createObjectURL(e.data);
      [front, back] = [back, front];
      image.src = imageUrl[front];
    }
  } else {
    wsocket.onmessage = function (e) {
      urlCreator.revokeObjectURL(imageUrl[back]);
      imageUrl[back] = urlCreator.createObjectURL(e.data);
      [front, back] = [back, front];
      image.src = imageUrl[front];
    }
  }
  //wsocket.onopen = () => log("Image stream open");
  //wsocket.onerror = (_) => log("Image stream error");
  //wsocket.onclose = (e) => log(`Image stream closed ${e.code}`);
}



    }
  }
  