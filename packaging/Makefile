ROOT ?= /deploy
APP   = vatools

# This target is called by the sw installer on the target machine.  Does
# what is required to install the new software and activate it.
install:
	mkdir -p /mnt/faces/; chmod oug+rw /mnt/faces/
	mkdir -p $(ROOT)/$(APP).next
	tar zxf package.tar.gz -C $(ROOT)/$(APP).next
	-rm -rf $(ROOT)/$(APP).prev
	( /bin/bash -c 'if [ -e "/etc/init.d/run_pgd.sh" ]; then /etc/init.d/run_pgd.sh stop; sleep 5; fi' )
	-mv $(ROOT)/$(APP) $(ROOT)/$(APP).prev
	-mv $(ROOT)/$(APP).next $(ROOT)/$(APP)
	( cd $(ROOT)/; chown -R www-data:www-data $(APP) )
	( cp run_pgd.sh /etc/init.d/ ; cd /etc/init.d ; /usr/sbin/update-rc.d run_pgd.sh defaults )
	( /etc/init.d/run_pgd.sh start )

