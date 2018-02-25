#!/bin/bash
sudo cp `dirname $0`/libWeather.so /usr/lib/dde-dock/plugins/libWeather.so
killall dde-dock
