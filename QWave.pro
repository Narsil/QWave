TEMPLATE = subdirs

SUBDIRS = core \
	waveclient \
	waveserver \
        robots \
        robots/echoey

waveclient.depends = core
waveserver.depends = core
robots.depends = core
echoey.depends = robots
