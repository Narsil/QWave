TEMPLATE = subdirs

SUBDIRS = core \
	waveclient \
	waveserver \
        robots \
        robots/echoey \
	tools

waveclient.depends = core
waveserver.depends = core tools
robots.depends = core
echoey.depends = robots
