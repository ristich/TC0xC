import irc.bot
import re
import sqlite3
from datetime import datetime

server = 'irl.depaul.edu'
port = 6667
channel = '#bb23'
nickname = 'TagBot'
max_damage = 254
critical_percentage = 0.1


class IRCBot(irc.bot.SingleServerIRCBot):
    def __init__(self):
        irc.bot.SingleServerIRCBot.__init__(self, [(server, port)], nickname, nickname)
        self.nickname = nickname
        self.conn = sqlite3.connect('hits.db')
        self.create_table()

    def create_table(self):
        cursor = self.conn.cursor()
        cursor.execute('''CREATE TABLE IF NOT EXISTS hits
                          (id INTEGER PRIMARY KEY AUTOINCREMENT,
                          user_id TEXT,
                          attacker_id TEXT,
                          damage_points INTEGER,
                          timestamp TEXT)''')
        self.conn.commit()

    def on_welcome(self, connection, event):
        print(f"Connected to {server}:{port}")
        connection.join(channel)
        print(f"Joined channel: {channel}")

    def on_pubmsg(self, connection, event):
        user = event.source.nick
        message = event.arguments[0]
        self.process_message(user, message)

    def process_message(self, user, message):
        regex = r'received (\d+) points of damage from (.+)'
        match = re.match(regex, message)
        if match:
            damage_points = int(match.group(1))
            attacker = match.group(2)
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            self.store_hit(user, attacker, damage_points, timestamp)
            total_damage = self.get_total_damage(user)
            message = f"{user} has taken a total of {total_damage} points of damage."
            print(message)
            self.connection.privmsg(channel, message)
            self.check_jeopardy(channel, user, total_damage)
        elif message.startswith('!leaderboard'):
            self.print_leaderboard(channel)
        elif message.startswith('!damage'):
            self.print_damage(user)

    def store_hit(self, user, attacker, damage_points, timestamp):
        cursor = self.conn.cursor()
        cursor.execute("INSERT INTO hits (user_id, attacker_id, damage_points, timestamp) VALUES (?, ?, ?, ?)",
                       (user, attacker, damage_points, timestamp))
        self.conn.commit()
        print(f"Stored hit: user={user}, attacker={attacker}, damage={damage_points}, timestamp={timestamp}")

    def get_total_damage(self, user):
        cursor = self.conn.cursor()
        cursor.execute("SELECT SUM(damage_points) FROM hits WHERE user_id=?", (user,))
        result = cursor.fetchone()
        total_damage = result[0] if result[0] else 0
        return total_damage

    def check_jeopardy(self, channel, user, total_damage):
        remaining_health = max_damage - total_damage
        critical_threshold = max_damage * critical_percentage
        if remaining_health <= critical_threshold:
            message = f"{user} is in jeopardy of being eliminated!"
            print(message)
            self.connection.privmsg(channel, message)

    def print_leaderboard(self, channel):
        cursor = self.conn.cursor()

        # Players who have dealt the most damage
        cursor.execute("SELECT attacker_id, SUM(damage_points) AS total_damage FROM hits GROUP BY attacker_id ORDER BY total_damage DESC")
        damage_dealt_leaderboard = cursor.fetchall()

        message_header = "Damage Dealt Leaderboard:"
        self.connection.privmsg(channel, message_header)

        for rank, (attacker_id, total_damage) in enumerate(damage_dealt_leaderboard, start=1):
            message = f"{rank}. {attacker_id}: {total_damage} damage"
            self.connection.privmsg(channel, message)

        print(message_header)
        for rank, (attacker_id, total_damage) in enumerate(damage_dealt_leaderboard, start=1):
            print(f"{rank}. {attacker_id}: {total_damage} damage")

        cursor.execute("SELECT user_id, SUM(damage_points) AS total_damage FROM hits GROUP BY user_id ORDER BY total_damage DESC")
        leaderboard = cursor.fetchall()

        message_header = "Damage Taken Leaderboard:"
        self.connection.privmsg(channel, message_header)

        for rank, (user_id, total_damage) in enumerate(leaderboard, start=1):
           message = f"{rank}. {user_id}: {total_damage} damage"
           self.connection.privmsg(channel, message)

        print(message_header)
        for rank, (user_id, total_damage) in enumerate(leaderboard, start=1):
            print(f"{rank}. {user_id}: {total_damage} damage")

    def print_damage(self, user):
       total_damage = self.get_total_damage(user)
       message = f"You have taken a total of {total_damage} points of damage."
       print(message)
       self.connection.privmsg(user, message)


# Create an instance of the IRCBot class and start the bot
bot = IRCBot()
print(f"Connecting to {server}:{port}...")
bot.start()

