<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LED Control</title>
</head>
<body>
    <h1 style="color:blue;">IoT LED Control</h1>
    <h4>Control the LED by clicking a button:</h4>
    
    <!-- Form for LED control -->
    <form method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
        Click to turn ON: <input type="submit" name="fname" value="on">
        <p></p>
        Click to Turn OFF: <input type="submit" name="fname" value="off">
    </form>
    <!-- --------------------- -->
    
    <?php
    // Handle form submission
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
        // Get the state from the button click
        $state = $_POST['fname'];

        // Display the last clicked button
        echo "<p>Last time you clicked was: $state</p>";

        // Write the state to results.txt
        $myfile = fopen("results.txt", "w") or die("Unable to open file!");
        fwrite($myfile, $state);
        fclose($myfile);
    }
    ?>
</body>
</html>
