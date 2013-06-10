/* $Id$ */

/*
 * Copyright (c) 2013
 *      Tim Rightnour.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of Tim Rightnour may not be used to endorse or promote 
 *    products derived from this software without specific prior written 
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TIM RIGHTNOUR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL TIM RIGHTNOUR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "gnhast.h"
#include "common.h"
#include "commands.h"
#include "cmds.h"
#include "gncoll.h"
#include "gnhastd.h"

extern TAILQ_HEAD(, _device_t) alldevs;

/**
   \file cmdhandler.c
   \brief Common command handlers
   \author Tim Rightnour
*/

extern struct event_base *base;
extern argtable_t argtable[];

/** The command table */
commands_t commands[] = {
	{"chg", cmd_change, 0},
	{"reg", cmd_register, 0},
	{"upd", cmd_update, 0},
	{"feed", cmd_feed, 0},
	{"ldevs", cmd_list_devices, 0},
	{"ask", cmd_ask_device, 0},
	{"cactiask", cmd_cactiask_device, 0},
	{"disconnect", cmd_disconnect, 0},
	{"client", cmd_client, 0},
};

/** The size of the command table */
const size_t commands_size = sizeof(commands) / sizeof(commands_t);


/**
	\brief Initialize the commands table
*/

void init_commands(void)
{
	qsort((char *)commands, commands_size, sizeof(commands_t),
	    compare_command);
}


/**
	\brief Handle a command from the network
	\param command command to execute
	\param args arguments to command
*/

int parsed_command(char *command, pargs_t *args, void *arg)
{
	commands_t *asp, dummy;
	char *cp;
	int ret;

	for (cp=command; *cp; cp++)
		*cp = tolower(*cp);

	dummy.name = command;
	asp = (commands_t *)bsearch((void *)&dummy, (void *)commands,
	    commands_size, sizeof(commands_t), compare_command);

	if (asp) {
		ret = asp->func(args, arg);
		return(ret);
	} else {
		return(-1); /* command not found */
	}
}

/* ================================ */
/* Handlers */

/**
	\brief Handle a update device command
	\param args The list of arguments
	\param arg void pointer to client_t of provider
*/

int cmd_update(pargs_t *args, void *arg)
{
	int i;
	int hadnodata = 0;
	device_t *dev;
	char *uid=NULL;
	client_t *client = (client_t *)arg;

	/* loop through the args and find the UID */
	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_UID:
			uid = args[i].arg.c;
			break;
		}
	}
	if (!uid) {
		LOG(LOG_ERROR, "update without UID");
		return(-1);
	}
	dev = find_device_byuid(uid);
	if (!dev) {
		LOG(LOG_ERROR, "UID:%s doesn't exist", uid);
		return(-1);
	}

	/* check if the device was unknown at the time */
	if (dev->flags & DEVFLAG_NODATA)
		hadnodata = 1;

	/* Ok, we got one, now lets update it's data */

	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_SWITCH:
			store_data_dev(dev, DATALOC_DATA, &args[i].arg.i);
			break;
		case SC_LUX:
		case SC_HUMID:
		case SC_TEMP:
		case SC_DIMMER:
		case SC_PRESSURE:
		case SC_SPEED:
		case SC_DIR:
		case SC_MOISTURE:
		case SC_WETNESS:
		case SC_VOLTAGE:
		case SC_WATT:
		case SC_AMPS:
			store_data_dev(dev, DATALOC_DATA, &args[i].arg.d);
			break;
		case SC_COUNT:
			store_data_dev(dev, DATALOC_DATA, &args[i].arg.u);
			break;
		case SC_WATTSEC:
			store_data_dev(dev, DATALOC_DATA, &args[i].arg.ll);
			break;
		}
	}
	(void)time(&dev->last_upd);
	client->updates++;

	/* Always run handler on first update */
	if (dev->handler != NULL && (device_watermark(dev) != 0 || hadnodata))
		run_handler_dev(dev);

	return(0);
}

/**
   \brief Handle a change device command
   \param args The list of arguments
   \param arg void pointer to client_t of provider
*/

int cmd_change(pargs_t *args, void *arg)
{
	int i;
	device_t *dev;
	char *uid=NULL;
	client_t *client = (client_t *)arg;
	struct evbuffer *send;

	/* loop through the args and find the UID */
	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_UID:
			uid = args[i].arg.c;
			break;
		}
	}
	if (!uid) {
		LOG(LOG_ERROR, "update without UID");
		return(-1);
	}
	dev = find_device_byuid(uid);
	if (!dev) {
		LOG(LOG_ERROR, "UID:%s doesn't exist", uid);
		return(-1);
	}

	send = evbuffer_new();
	/* The command to change is "chg" */
	evbuffer_add_printf(send, "chg ");

	/* fill in the details */
	evbuffer_add_printf(send, "%s:%s", ARGNM(SC_UID), dev->uid);

	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_SWITCH:
			evbuffer_add_printf(send, " %s:%d",
					    ARGNM(args[i].cword),
					    args[i].arg.i);
			break;
		case SC_LUX:
		case SC_HUMID:
		case SC_TEMP:
		case SC_DIMMER:
		case SC_PRESSURE:
		case SC_SPEED:
		case SC_DIR:
		case SC_MOISTURE:
		case SC_WETNESS:
		case SC_VOLTAGE:
		case SC_WATT:
		case SC_AMPS:
			evbuffer_add_printf(send, " %s:%f",
					    ARGNM(args[i].cword),
					    args[i].arg.d);
			store_data_dev(dev, DATALOC_DATA, &args[i].arg.d);
			break;
		case SC_COUNT:
			evbuffer_add_printf(send, " %s:%d",
					    ARGNM(args[i].cword),
					    args[i].arg.u);
			break;
		case SC_WATTSEC:
			evbuffer_add_printf(send, " %s:%jd",
					    ARGNM(args[i].cword),
					    args[i].arg.ll);
			break;
		}
	}
	if (dev->collector == NULL) {
		LOG(LOG_ERROR, "Got chg for uid:%s, but no collector",
		    dev->uid);
		return(-1);
	}
	/* and send it on it's way */
	evbuffer_add_printf(send, "\n");
	bufferevent_write_buffer(dev->collector->ev, send);
	return(0);
}

/**
	\brief Handle a register device command
	\param args The list of arguments
	\param arg void pointer to client_t of provider
*/

int cmd_register(pargs_t *args, void *arg)
{
	int i, new=0;
	uint8_t devtype=0, proto=0, subtype=0, scale=0;
	char *uid=NULL, *name=NULL, *rrdname=NULL;
	device_t *dev;
	client_t *client = (client_t *)arg;

	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_UID:
			uid = strdup(args[i].arg.c);
			break;
		case SC_NAME:
			name = strdup(args[i].arg.c);
			break;
		case SC_RRDNAME:
			rrdname = strndup(args[i].arg.c, 19);
			break;
		case SC_DEVTYPE:
			devtype = (uint8_t)args[i].arg.i;
			break;
		case SC_PROTO:
			proto = (uint8_t)args[i].arg.i;
			break;
		case SC_SUBTYPE:
			subtype = (uint8_t)args[i].arg.i;
			break;
		case SC_SCALE:
			scale = (uint8_t)args[i].arg.i;
			break;
		}
	}

	if (uid == NULL) {
		LOG(LOG_ERROR, "Got register command without UID");
		return(-1); /* MUST have UID */
	}

	LOG(LOG_DEBUG, "Register device: uid=%s name=%s rrd=%s type=%d proto=%d subtype=%d",
	    uid, (name) ? name : "NULL", (rrdname) ? rrdname : "NULL",
	    devtype, proto, subtype);

	dev = find_device_byuid(uid);
	if (dev == NULL) {
		LOG(LOG_DEBUG, "Creating new device for uid %s", uid);
		if (subtype == 0 || devtype == 0 || proto == 0 || name == NULL) {
			LOG(LOG_ERROR, "Attempt to register new device without full specifications");
			return(-1);
		}
		if (rrdname == NULL)
			rrdname = strndup(name, 19);
		dev = smalloc(device_t);
		dev->uid = uid;
		new = 1;
	} else
		LOG(LOG_DEBUG, "Updating existing device uid:%s", uid);
	dev->name = name;
	dev->rrdname = rrdname;
	dev->type = devtype;
	dev->proto = proto;
	dev->subtype = subtype;
	dev->scale = scale;
	(void)time(&dev->last_upd);
	if (dev->collector != NULL && dev->collector != client)
		LOG(LOG_ERROR, "Device uid:%s has a collector, but a new one registered it!", dev->uid);
	dev->collector = client;

	/* now, update the client_t */
	if (TAILQ_EMPTY(&client->devices)) {
		TAILQ_INIT(&client->devices);
		client->provider = 1; /* this client is a provider */
	}
	if (!(dev->onq & DEVONQ_CLIENT)) {
		TAILQ_INSERT_TAIL(&client->devices, dev, next_client);
		dev->onq |= DEVONQ_CLIENT;
	}
	dev->collector = client;
	
	if (new)
		insert_device(dev);

	return(0);
}

/**
   \brief Timer callback to update the feed request
   \param nada used for file descriptor
   \param what why did we fire?
   \param arg void pointer to client_t of connection
*/

void feeddata_cb(int nada, short what, void *arg)
{
	client_t *client = (client_t *)arg;
	wrap_device_t *wrap;
	time_t now, diff;

	(void)time(&now);
	TAILQ_FOREACH(wrap, &client->wdevices, next) {
		diff = now - wrap->last_fired;
		if (diff / wrap->rate >= 1) { /* this dev is ready */
			gn_update_device(wrap->dev, GNC_UPD_RRDNAME |
					 GNC_UPD_SCALE(wrap->scale),
					 client->ev);
			wrap->last_fired = now;
		}
	}
}

/**
   \brief Handle a feed device command
   \param args The list of arguments
   \param arg void pointer to client_t of provider
*/

int cmd_feed(pargs_t *args, void *arg)
{
	int i, rate=60, lc, scale=0;
	char *uid=NULL;
	struct timeval secs = {0, 0};
	device_t *dev;
	wrap_device_t *wrap;
	client_t *client = (client_t *)arg;

	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_UID:
			uid = args[i].arg.c;
			break;
		case SC_RATE:
			rate = args[i].arg.i;
			break;
		case SC_SCALE:
			scale = args[i].arg.i;
			break;
		}
	}
	secs.tv_sec = rate;
	dev = find_device_byuid(uid);
	if (dev == NULL)
		return -1;
	add_wrapped_device(dev, client, rate, scale);
	if (client->tev == NULL || !event_initialized(client->tev)) {
		client->tev = event_new(base, -1, EV_PERSIST, feeddata_cb,
					client);
		event_add(client->tev, &secs);
		client->timer_gcd = rate;
	} else {
		TAILQ_FOREACH(wrap, &client->wdevices, next) {
			lc = gcd(wrap->rate, client->timer_gcd);
			client->timer_gcd = lc;
		}
		secs.tv_sec = client->timer_gcd;
		event_add(client->tev, &secs); /* update with new lcm */
	}
	return 0;
}

/**
   \brief Handle a client command
   \param args The list of arguments
   \param arg void pointer to client_t of provider
*/

int cmd_client(pargs_t *args, void *arg)
{
	int i;
	char *cli=NULL;
	client_t *client = (client_t *)arg;

	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_CLIENT:
			cli = args[i].arg.c;
			break;
		}
	}

	if (cli == NULL)
		return -1;
	client->name = strdup(cli);
	LOG(LOG_NOTICE, "Client %s registered as %s",
	    client->addr ? client->addr : "unknown", cli);
	return 0;
}

/**
	\brief Handle a list devices command
	\param args The list of arguments
	\param arg void pointer to client_t of connection
*/

int cmd_list_devices(pargs_t *args, void *arg)
{
	int i, devtype, proto, subtype;
	device_t *dev;
	char *uid = NULL;
	client_t *client = (client_t *)arg;
	struct evbuffer *send;

	devtype = proto = subtype = 0;

	/* the arguments to this command are used as qualifiers for the
	   device list search */
	for (i=0; args[i].cword != -1; i++) {
		switch (args[i].cword) {
		case SC_UID:
			uid = args[i].arg.c;
			break;
		case SC_DEVTYPE:
			devtype = args[i].arg.i;
			break;
		case SC_PROTO:
			proto = args[i].arg.i;
			break;
		case SC_SUBTYPE:
			subtype = args[i].arg.i;
			break;
		}
	}
	TAILQ_FOREACH(dev, &alldevs, next_all) {
		if (uid && strcmp(uid, dev->uid) != 0)
			continue;
		if (devtype && devtype != dev->type)
			continue;
		if (proto && proto != dev->proto)
			continue;
		if (subtype && subtype != dev->subtype)
			continue;
		gn_register_device(dev, client->ev);
	}
	send = evbuffer_new();
	evbuffer_add_printf(send, "endldevs\n");
	bufferevent_write_buffer(client->ev, send);
	evbuffer_free(send);
	return 0;
}

/**
	\brief Handle a ask device command
	\param args The list of arguments
	\param arg void pointer to client_t of connection
*/

int cmd_ask_device(pargs_t *args, void *arg)
{
	int i, scale=0;
	device_t *dev;
	char *uid = NULL;
	client_t *client = (client_t *)arg;

	/* check for a scale argument first */
	for (i=0; args[i].cword != -1; i++)
		if (args[i].cword == SC_SCALE)
			scale = args[i].arg.i;

	for (i=0; args[i].cword != -1; i++) {
		if (args[i].cword == SC_UID) {
			uid = args[i].arg.c;
			dev = find_device_byuid(uid);
			if (dev == NULL)
				continue;
			gn_update_device(dev, GNC_UPD_NAME|GNC_UPD_RRDNAME|
					 GNC_UPD_SCALE(scale), client->ev);
		}
	}
	return 0;
}

/**
   \brief Handle a ask device command
   \param args The list of arguments
   \param arg void pointer to client_t of connection
*/

int cmd_cactiask_device(pargs_t *args, void *arg)
{
	int i, scale=0;
	device_t *dev;
	char *uid = NULL;
	client_t *client = (client_t *)arg;

	/* check for a scale argument first */
	for (i=0; args[i].cword != -1; i++)
		if (args[i].cword == SC_SCALE)
			scale = args[i].arg.i;

	for (i=0; args[i].cword != -1; i++) {
		if (args[i].cword == SC_UID) {
			uid = args[i].arg.c;
			dev = find_device_byuid(uid);
			if (dev == NULL)
				continue;
			gn_update_device(dev, GNC_UPD_CACTI|
					 GNC_UPD_SCALE(scale), client->ev);
		}
	}
	return 0;
}

/**
   \brief Close a connection down
   \param args The list of arguments (ignored)
   \param arg void pointer to client_t of connection
*/

int cmd_disconnect(pargs_t *args, void *arg)
{
	client_t *client = (client_t *)arg;

	/* set close on empty, set watermark, then enable the write callback */
	client->close_on_empty = 1;
	bufferevent_setwatermark(client->ev, EV_WRITE, 0, 0);
	bufferevent_setcb(client->ev, buf_read_cb, buf_write_cb,
			  buf_error_cb, client);
	bufferevent_enable(client->ev, EV_READ|EV_WRITE);
	LOG(LOG_NOTICE, "Client %s from %s requested disconnect",
	    client->name ? client->name : "generic",
	    client->addr ? client->addr : "unknown");
}
