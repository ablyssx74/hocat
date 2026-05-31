name			$(NAME)
version			$(VERSION)-1
architecture	$(ARCH)
summary 		"HoCat"
description 	"HoCat - Native GUI wrapper for blink socat"
packager		"ablyss <hocat@epluribusunix.net>"
vendor			"epluribusunix.net Project"
licenses {
	"MIT"
}
copyrights {
	"$(YEAR) ablyss"
}
provides {
	$(NAME) = $(VERSION)-1
}
requires {
	haiku
	blink
}	
urls {
	"https://github.com/ablyssx74/hocat"
}
