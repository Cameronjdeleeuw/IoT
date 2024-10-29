<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RGB LED Controller</title>
</head>
<body>
    <h2>RGB LED Controller</h2>
    <form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>">
        <!-- Slider for Red -->
        <label for="redSlider">Red (0-255):</label>
        <input type="range" id="redSlider" name="red" min="0" max="255" value="0" oninput="this.nextElementSibling.value = this.value">
        <output>0</output>
        <br><br>

        <!-- Slider for Green -->
        <label for="greenSlider">Green (0-255):</label>
        <input type="range" id="greenSlider" name="green" min="0" max="255" value="0" oninput="this.nextElementSibling.value = this.value">
        <output>0</output>
        <br><br>

        <!-- Slider for Blue -->
        <label for="blueSlider">Blue (0-255):</label>
        <input type="range" id="blueSlider" name="blue" min="0" max="255" value="0" oninput="this.nextElementSibling.value = this.value">
        <output>0</output>
        <br><br>

        <!-- Submit button -->
        <input type="submit" value="Submit Values">
    </form>

    <?php
    // Only process the form and write to the file if the request method is POST
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
        // Get the values from the sliders
        $red_value = isset($_POST['red']) ? $_POST['red'] : 0;
        $green_value = isset($_POST['green']) ? $_POST['green'] : 0;
        $blue_value = isset($_POST['blue']) ? $_POST['blue'] : 0;

        // Display the selected values on the page
        echo "<p>Red Value: " . $red_value . "</p>";
        echo "<p>Green Value: " . $green_value . "</p>";
        echo "<p>Blue Value: " . $blue_value . "</p>";

        // Write the values to the file
        $file = fopen("getRGB.txt", "w") or die("Unable to open file!");
        fwrite($file, "R=" . $red_value);
        fwrite($file, "G=" . $green_value);
        fwrite($file, "B=" . $blue_value);
        fclose($file);
        
        echo "<p>Values saved to getRGB.txt</p>";
    }
    ?>
</body>
</html>
