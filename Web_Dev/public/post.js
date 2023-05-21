// Call fetchAndDisplayPosts() on page load
document.addEventListener('DOMContentLoaded', function () {
  fetchAndDisplayPosts();
});

// Fetch data from the server and display all posts
function fetchAndDisplayPosts() {
  console.log("Fetch and display posts")
  fetch('/posts')
    .then(response => response.json())
    .then(posts => {
      console.log("Fetched posts:", posts);

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

        // Create the text boxes element
        var textBoxesElement = document.createElement("div");
        textBoxesElement.className = "text-boxes";
        var postTextElement = document.createElement("div");
        postTextElement.className = "post-text";
        postTextElement.textContent = post.PostTextArea;
        var postFileElement = document.createElement("div");
        postFileElement.className = "post-file";

        if (post.FilePath) {
          // Display the file content
          console.log("Fetching file:", post.FilePath);

          fetch(post.FilePath)
            .then(response => response.text())
            .then(fileContent => {
              postFileElement.textContent = fileContent;
            })
            .catch(error => console.error(error));
        } else {
          // Handle the case where no file is available
          postFileElement.textContent = "No file attached";
        }

        // Create the like counter element
        var likeCounterElement = document.createElement("div");
        likeCounterElement.className = "like-counter";
        likeCounterElement.setAttribute("data-count", "0");
        var likeText = document.createElement("p");
        likeText.textContent = "0";
        var likeButton = document.createElement("div");
        likeButton.className = "like-button";
        var likeImage = document.createElement("img");
        likeImage.src = "unliked.png";
        likeImage.id = "like-button";
        likeImage.onclick = incrementLike;
        likeButton.appendChild(likeImage);
        likeCounterElement.appendChild(likeButton);
        likeCounterElement.appendChild(likeText);

        // Append all elements to the post container
        newPostContainer.appendChild(usernameElement);
        textBoxesElement.appendChild(postTextElement);
        textBoxesElement.appendChild(postFileElement);
        newPostContainer.appendChild(textBoxesElement);
        newPostContainer.appendChild(likeCounterElement);

        // Append the post container to the post element
        postElement.appendChild(newPostContainer);
      });
    })
    .catch(error => console.error(error));
}

function createNewElement() {
  document.getElementById("submitButton").addEventListener("click", function (event) {
    event.preventDefault(); // Prevent the default form submission

    // Get the form input values
    var postTextarea = document.getElementById("post-textarea").value;
    var postFileInput = document.getElementById("post-file");

    // Create a new FormData object
    var formData = new FormData();
    formData.append('PostTextArea', postTextarea);
    formData.append('PostFile', postFileInput.files[0]);

    // Send the form data to the server using fetch
    fetch('/posts', {
      method: 'POST',
      body: formData
    })
      .then(response => response.json())
      .then(() => {
        // Clear the form input fields
        document.getElementById("post-textarea").value = '';
        document.getElementById("post-file").value = '';

        // Fetch and display all posts, including the newly created one
        fetchAndDisplayPosts();
      })
      .catch(error => console.error(error));
  });
}

// Initialize the code for creating new elements
createNewElement();