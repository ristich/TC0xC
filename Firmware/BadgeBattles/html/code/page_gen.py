import sqlite3
import datetime
import boto3

# Function to generate the HTML leaderboard table rows
def generate_table_rows(cursor):
    table_rows = ""
    for rank, (user_id, total_damage) in enumerate(cursor, start=1):
        table_rows += f"<tr>"
        table_rows += f"<td>{rank}.</td>"
        table_rows += f"<td>{user_id}</td>"
        table_rows += f"<td>{total_damage}</td>"
        table_rows += f"</tr>"
    return table_rows

# Connect to the SQLite database
conn = sqlite3.connect('hits.db')
cursor = conn.cursor()

# Retrieve the leaderboard data for damage dealt
cursor.execute("SELECT attacker_id, SUM(damage_points) AS total_damage FROM hits GROUP BY attacker_id ORDER BY total_damage DESC")
damage_dealt_leaderboard = cursor.fetchall()

# Retrieve the leaderboard data for damage taken
cursor.execute("SELECT user_id, SUM(damage_points) AS total_damage FROM hits GROUP BY user_id ORDER BY total_damage DESC")
damage_taken_leaderboard = cursor.fetchall()

# Generate the HTML content
html_content = """
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>BadgeBattles</title>
  <style>
    body {
      background-color: #000000;
      color: #ffffff;
      font-family: monospace;
      text-align: center;
      padding: 40px;
    }

    pre {
      font-size: 24px;
      line-height: 1.6;
    }

    table {
      margin: 0 auto;
      border-collapse: collapse;
    }

    th, td {
      padding: 10px;
      border: 1px solid white;
    }

    th {
      background-color: #333333;
      color: white;
    }

    .critical {
      color: red;
      animation: blink 1s infinite;
    }

    @keyframes blink {
      50% {
        opacity: 0;
      }
    }
  </style>
</head>

<body>
  <pre>
__________             .___            __________         __    __  .__                 
\______   \_____     __| _/ ____   ____\______   \_____ _/  |__/  |_|  |   ____   ______
 |    |  _/\__  \   / __ | / ___\_/ __ \|    |  _/\__  \\   __\   __\  | _/ __ \ /  ___/
 |    |   \ / __ \_/ /_/ |/ /_/  >  ___/|    |   \ / __ \|  |  |  | |  |_\  ___/ \___ \ 
 |______  /(____  /\____ |\___  / \___  >______  /(____  /__|  |__| |____/\___  >____  >
        \/      \/      \/_____/      \/       \/      \/                     \/     \/ 



</pre>
"""

# Append the damage dealt leaderboard to the HTML content
current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
html_content += f"<h1> Last Updated: {current_time}</h1>"
html_content += "<h2>Damage Dealt Leaderboard</h2>"
html_content += "<table>"
html_content += "<tr><th>Rank</th><th>Attacker ID</th><th>Total Damage</th></tr>"
html_content += generate_table_rows(damage_dealt_leaderboard)
html_content += "</table>"

# Append the damage taken leaderboard to the HTML content
html_content += "<h2>Damage Taken Leaderboard</h2>"
html_content += "<table>"
html_content += "<tr><th>Rank</th><th>User ID</th><th>Total Damage</th></tr>"
html_content += generate_table_rows(damage_taken_leaderboard)
html_content += "</table>"

# Close the HTML tags
html_content += """
</body>
</html>
"""
def upload_to_s3(bucket_name, file_name):
    s3 = boto3.client("s3")

    try:
        s3.upload_file(file_name, bucket_name, "index.html", ExtraArgs={"ContentType": "text/html", "ACL": "public-read"})
        print("Uploaded index.html to S3 successfully.")
    except Exception as e:
        print(f"Error uploading index.html to S3: {str(e)}")

bucket_name = "badgebattles" 
# Write the HTML content to the index.html file
with open("../index.html", "w") as file:
    file.write(html_content)


upload_to_s3(bucket_name, "../index.html")
