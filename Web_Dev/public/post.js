document.addEventListener('DOMContentLoaded', function () {
  fetchAndDisplayPosts();
  createNewElement();
});

function fetchAndDisplayPosts() {
  console.log("Fetch and display posts");
  fetch('/posts')
    .then(response => response.json())
    .then(posts => {
      console.log("Fetched posts:", posts);
      posts = posts.reverse();
      var postElement = document.getElementById("post");
      postElement.innerHTML = '';

      posts.forEach(post => {
        var newPostContainer = document.createElement("div");
        newPostContainer.className = "post-container";

        var usernameElement = document.createElement("div");
        usernameElement.className = "username";
        var usernameText = document.createElement("p");
        usernameText.textContent = "User";
        usernameElement.appendChild(usernameText);

        var textBoxesElement = document.createElement("div");
        textBoxesElement.className = "text-boxes";
        var postTextElement = document.createElement("div");
        postTextElement.className = "post-text";
        postTextElement.textContent = post.PostTextArea;
        var postFileElement = document.createElement("div");
        postFileElement.className = "PostFile";

        if (post.FilePath) {
          console.log("Fetching file:", post.FilePath);
          fetch(post.FilePath)
            .then(response => response.text())
            .then(fileContent => {
              postFileElement.textContent = fileContent;
            })
            .catch(error => console.error(error));
        }

        var likeCounterElement = document.createElement("div");
        likeCounterElement.className = "like-counter";
        likeCounterElement.setAttribute("data-count", post.likes); // Use the actual like count from the post
        var likeText = document.createElement("p");
        likeText.textContent = post.likes; // Display the actual like count from the post
        var likeButton = document.createElement("div");
        likeButton.className = "like-button";
        var likeImage = document.createElement("img");
        likeImage.src = "unliked.png";
        likeImage.id = "like-button";
        likeImage.onclick = incrementLike;
        likeButton.appendChild(likeImage);
        likeCounterElement.appendChild(likeButton);
        likeCounterElement.appendChild(likeText);

        newPostContainer.appendChild(usernameElement);
        textBoxesElement.appendChild(postTextElement);
        textBoxesElement.appendChild(postFileElement);
        newPostContainer.appendChild(textBoxesElement);
        newPostContainer.appendChild(likeCounterElement);

        postElement.appendChild(newPostContainer);
      });
    })
    .catch(error => console.error(error));
}

function createNewElement() {
  var newPostForm = document.getElementById("newPostForm");
  newPostForm.addEventListener("submit", handleSubmit);

  function handleSubmit(event) {
    event.preventDefault();
    var postTextarea = document.getElementById("post-textarea").value;
    var postFileInput = document.getElementById("PostFile");

    var formData = new FormData();
    formData.append('PostTextArea', postTextarea);

    if (postFileInput.files.length > 0) {
      formData.append('PostFile', postFileInput.files[0], postFileInput.files[0].name);
    }

    fetch('/posts', {
      method: 'POST',
      body: formData
    })
      .then(response => response.json())
      .then(() => {
        document.getElementById("post-textarea").value = '';
        document.getElementById("PostFile").value = '';
        fetchAndDisplayPosts();
      })
      .catch(error => console.error(error));
  }
}