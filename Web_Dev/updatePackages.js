const { exec } = require('child_process');

exec('npm update', (error, stdout, stderr) => {
  if (error) {
    console.error(`Error occurred: ${error.message}`);
    return;
  }
  if (stderr) {
    console.error(`Command execution error: ${stderr}`);
    return;
  }
  console.log(`Packages updated:\n${stdout}`);
});