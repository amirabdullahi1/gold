#!/usr/bin/env python3

# usage: ./gen-message.py <csv-file-name>
import re
import datetime
import fileinput


template = """Nuttin'
Honey
"""

dirpath = "messages/"


def main():
    today = datetime.date.today()
    six_weeks_from_today = today + datetime.timedelta(weeks=6)

    today = today.strftime("%B %d %Y")
    six_weeks_from_today = six_weeks_from_today.strftime("%B %d %Y")

    for line in fileinput.input():
        pass


if __name__ == "__main__":
    main()
