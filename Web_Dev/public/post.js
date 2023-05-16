function createNewElement() {
  document.getElementById("submitButton").addEventListener("click", function(event) {
    event.preventDefault(); // Prevent the default form submission
  
    // Get the form input values
    var postTextarea = document.getElementById("post-textarea").value;
    var postFile = document.getElementById("post-file").value;
  
    // Create a new post object with the form data
    var newPost = {
      PostTextArea: postTextarea,
      PostFile: postFile
    };
  
    // Send the new post data to the server
    fetch('/posts', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(newPost)
    })
      .then(response => response.json())
      .then(createdPost => {
        // Clear the form input fields
        document.getElementById("post-textarea").value = '';
        document.getElementById("post-file").value = '';
  
        // Fetch and display all posts, including the newly created one
        fetchAndDisplayPosts();
      })
      .catch(error => console.error(error));
  });
}

// Fetch data from the server and display all posts
function fetchAndDisplayPosts() {
  fetch('/posts')
    .then(response => response.json())
    .then(posts => {
      // Reverse the order of posts array
      posts = posts.reverse();

      // Clear the existing content in the post element
      var postElement = document.getElementById("post");
      postElement.innerHTML = '';

      // Iterate over each post and create the post container with data
      posts.forEach(post => {
        // Create a new post container
        var newPostContainer = document.createElement("div");
        newPostContainer.className = "post-container";

        // Create the username element
        var usernameElement = document.createElement("div");
        usernameElement.className = "username";
        var usernameText = document.createElement("p");
        usernameText.textContent = "User";
        usernameElement.appendChild(usernameText);

        // Create the post profile element
        // var postProfileElement = document.createElement("div");
        // postProfileElement.className = "post-profile";
        // var profileImage = document.createElement("img");
        // profileImage.src = "picture.png";
        // postProfileElement.appendChild(profileImage);

        // Create the like counter element
        var likeCounterElement = document.createElement("div");
        likeCounterElement.className = "like-counter";
        likeCounterElement.setAttribute("data-count", "0");
        var likeText = document.createElement("p");
        likeText.textContent = "0 likes";
        var likeButton = document.createElement("div");
        likeButton.className = "like-button";
        var likeImage = document.createElement("img");
        likeImage.src = "unliked.png";
        likeImage.id = "like-button";
        likeImage.onclick = incrementLike;
        likeButton.appendChild(likeImage);
        likeCounterElement.appendChild(likeText);
        likeCounterElement.appendChild(likeButton);

        // Create the text boxes element
        var textBoxesElement = document.createElement("div");
        textBoxesElement.className = "text-boxes";
        var postTextElement = document.createElement("p");
        postTextElement.className = "post-text";
        postTextElement.textContent = post.PostTextArea;
        var postFileElement = document.createElement("p");
        postFileElement.className = "post-file";
        postFileElement.textContent = post.PostFile;

        // Append all elements to the post container
        newPostContainer.appendChild(usernameElement);
        //newPostContainer.appendChild(postProfileElement);
        newPostContainer.appendChild(likeCounterElement);
        textBoxesElement.appendChild(postTextElement);
        textBoxesElement.appendChild(postFileElement);
        newPostContainer.appendChild(textBoxesElement);

        // Append the post container to the post element
        postElement.appendChild(newPostContainer);
      });
    })
    .catch(error => console.error(error));
}

// Call fetchAndDisplayPosts() on page load
fetchAndDisplayPosts();
