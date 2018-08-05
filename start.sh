clear
rm -rf ./bin_cgi/*
make
killall reg login upload md5
spawn-fcgi ./bin_cgi/login -p 30000
spawn-fcgi ./bin_cgi/reg -p 30001
spawn-fcgi ./bin_cgi/upload -p 30002
spawn-fcgi ./bin_cgi/md5 -p 30003

sudo cp ./nginx.conf /usr/local/nginx/conf/
sudo nginx -s stop
sudo nginx -s reload
sudo nginx

sudo redis-cli shutdown
sudo redis-server /mnt/hgfs/c++/work/cloud_server/conf/redis.conf

# fdfs_trackerd /etc/fdfs/tracker.conf.sample stop
# fdfs_storaged /etc/fdfs/storage.conf.sample stop
# fdfs_trackerd /etc/fdfs/tracker.conf.sample
# fdfs_storaged /etc/fdfs/storage.conf.sample

ps -aux|grep bin_cgi