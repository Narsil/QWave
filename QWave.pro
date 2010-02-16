TEMPLATE = subdirs

SUBDIRS = core \
	waveclient \
	waveserver \

waveclient.depends = core
waveserver.depends = core
