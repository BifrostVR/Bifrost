const fs = require('fs');
const path = require('path');

function generateUniqueFilePath(userFile) {
    if (!userFile || !userFile.originalname) {
        // Handle the case when no file is selected
        return '';
    }

    const timestamp = Date.now();
    const randomNumber = Math.floor(Math.random() * 1000000);
    const fileName = `file_${timestamp}_${randomNumber}`;

    const fileExtension = getFileExtension(userFile);
    const filePath = `uploads/${fileName}.${fileExtension}`;

    // Create the destination folder if it doesn't exist
    const folderPath = path.dirname(filePath);
    fs.mkdirSync(folderPath, { recursive: true });

    return filePath;
}

function getFileExtension(userFile) {
    if (typeof userFile === 'string') {
        // Extract the file extension from the file name
        const parts = userFile.split('.');
        if (parts.length === 1) {
            return ''; // No file extension found
        }
        return parts.pop().toLowerCase();
    } else if (userFile.originalname) {
        const parts = userFile.originalname.split('.');
        if (parts.length === 1) {
            return ''; // No file extension found
        }
        return parts.pop().toLowerCase();
    }

    // Handle other cases or return an appropriate value
    return ''; // Invalid input or no file extension found
}

function moveFileToDestination(file, destinationPath) {
    return new Promise((resolve, reject) => {
      const sourcePath = path.resolve(file.path); // Resolve the source path
      const destination = path.resolve(destinationPath); // Resolve the destination path
  
      fs.rename(sourcePath, destination, (err) => {
        if (err) {
          reject(err);
        } else {
          resolve();
        }
      });
    });
  }
  

module.exports = {
    generateUniqueFilePath,
    getFileExtension,
    moveFileToDestination
};