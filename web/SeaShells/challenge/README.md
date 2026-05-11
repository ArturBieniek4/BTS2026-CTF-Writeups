# SeaShells
## Points: 253

A team of shell collectors decided to make a website to document their findings. Maybe you can find something interesting too ;)  
Web: `https://<domain_from_below>`  
SSH: `ssh-<domain_from_below>`          
Do not forget about `ssh-` at beginning  
use [sc](https://github.com/CTFd/snicat) to bind it to the port on your machine:  
```
sc -b <local_port> <domain_url>
```
After that, you can connect to an instance with netcat:
```
ssh localhost -p <local_port>

