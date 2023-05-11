function createNewElement() {
  var newElement = document.createElement("div");
  /*VVVplaceholder for actually grabbing post attributes from serverVVV*/
  newElement.innerHTML = document.getElementById("post").innerHTML;

  // Find the post element and add it underneath post
  var container = document.getElementById("content");
  container.appendChild(newElement);
}
