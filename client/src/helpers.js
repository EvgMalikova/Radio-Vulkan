const Helper = {
    elementPosition(e) {
    const r = e.getBoundingClientRect();
    return {x: r.left + window.pageXOffset, y: r.top + window.pageYOffset};
  },
  
  appendScrollText(textarea, txt) {
    textarea.value += textarea.length ? txt : txt + '\n';
    textarea.scrollTop = textarea.scrollHeight;
  },
  
  mousePosition(mx, my, img) {
    const offset = elementPosition(img);
    return {x: Math.min(Math.max(mx - offset.x, 0), img.width - 1),
            y: Math.min(Math.max(my - offset.y, 0), img.height - 1)};
  }
};

export {Helper};