#!/usr/bin/env python
"""
Script for copying data from KPhotoAlbum index.xml to MySQL database.
"""

import sys
import getpass
import MySQLdb
from KPhotoAlbum import mysqldb
from KPhotoAlbum import xmldb

progressCounter = 0

def printn(msg):
	"""
	Print to stdout without new line.
	"""
	sys.stdout.write(msg)
	sys.stdout.flush()

def showProgress():
	global progressCounter
	if progressCounter % 100 == 0:
		printn('.')
	progressCounter += 1


def main(argv):
	try:
		me = argv[0]
	except:
		me = 'kpaxml2mysql.py'
		argv = [me]

	# Parse command line parameters
	if len(argv) < 3 or len(argv) > 5:
		print('Usage: ' + me +
		      ' xml_file db_name [username [password]]')
		return 1
	xml_file = argv[1]
	db_name = argv[2]
	if len(argv) < 4:
		username = getpass.getuser()
		print('Using username: ' + username)
	else:
		username = argv[3]
	if len(argv) < 5:
		password = getpass.getpass()
	else:
		password = argv[4]

	printn('Connecting to database...')
	db = MySQLdb.connect(db=db_name, user=username, passwd=password)
	destDb = mysqldb.MySQLDatabase(db)
	print('connected.')

	print('WARNING!')
	print('This script will mess up your database (' + db_name + ').')
	printn('Are you sure you want to continue? ')
	a = raw_input()
	if a[0] != 'y' and a[0] != 'Y':
		return 0

	printn('Do you want to clear old tables first? ')
	a = raw_input()
	doClear = (a[0] == 'y' or a[0] == 'Y')

	printn('Parsing the XML file...')
	try:
		srcDb = xmldb.XMLDatabase(xml_file)
	except xmldb.Error, (e):
		print('failed.')
		print(e)
		return 2
	print('parsed.')

	printn('Copying data to the MySQL database')
	if doClear:
		destDb.clear()
	global progressCounter
	progressCounter = 0
	destDb.feedFrom(srcDb, showProgress)
	print('copied.')

	return 0


if __name__ == '__main__':
	sys.exit(main(sys.argv))
