function incrementLike() {
    var likeButton = document.getElementById("like-button");
    var likeCounter = document.getElementById("like-counter");
    var count = parseInt(likeCounter.getAttribute("data-count"));
  
    if (likeButton.getAttribute("src") == "unliked.png") {
      likeButton.setAttribute("src", "liked.png");
      count++;
    } else {
      likeButton.setAttribute("src", "unliked.png");
      count--;
    }
  
    likeCounter.setAttribute("data-count", count);
    likeCounter.querySelector("p").innerHTML = count + " likes";
  }