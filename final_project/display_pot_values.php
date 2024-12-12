<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// Database configuration
$servername = "localhost";
$username   = "u413365777_db_camerondele";
$password   = "EEssu2024";
$dbname     = "u413365777_camerondeleeuw";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Prepare data for the graph
$chart_sql = "SELECT time_received, pot_value FROM potentiometer_data ORDER BY time_received ASC";
$chart_result = $conn->query($chart_sql);

$chart_data = [];
while ($row = $chart_result->fetch_assoc()) {
    $phpDate = strtotime($row['time_received']);
    $jsYear = date('Y', $phpDate);
    $jsMonth = date('n', $phpDate) - 1; // JavaScript months are 0-indexed
    $jsDay = date('j', $phpDate);
    $jsHour = date('H', $phpDate);
    $jsMinute = date('i', $phpDate);
    $jsSecond = date('s', $phpDate);
    $chart_data[] = "[new Date($jsYear, $jsMonth, $jsDay, $jsHour, $jsMinute, $jsSecond), " . $row['pot_value'] . "]";
}

$conn->close();
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Potentiometer Data Graph</title>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
        google.charts.load('current', { packages: ['corechart', 'line'] });
        google.charts.setOnLoadCallback(drawChart);

        function drawChart() {
            // Prepare chart data
            var data = google.visualization.arrayToDataTable([
                ['Time Received', 'Potentiometer Value'],
                <?php echo implode(",", $chart_data); ?>
            ]);

            // Chart options
            var options = {
                title: 'Potentiometer Values Over Time',
                hAxis: {
                    title: 'Time',
                    format: 'MMM dd, yyyy HH:mm', // Date format for X-axis
                    gridlines: { count: 10 }
                },
                vAxis: {
                    title: 'Potentiometer Value'
                },
                legend: { position: 'bottom' },
                curveType: 'function'
            };

            // Draw the chart
            var chart = new google.visualization.LineChart(document.getElementById('chart_div'));
            chart.draw(data, options);
        }
    </script>
</head>
<body>
    <h1>Potentiometer Data Graph</h1>
    <div id="chart_div" style="width: 100%; height: 500px;"></div>
</body>
</html>
