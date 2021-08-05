## SSL/TLS reverse proxy server

### Why do we need separate docker image?

The project is deployed via [Portainer][1] which [does not support][2] relative file pathes. \
Therefore, config files could not be copied to official container at docker-compose stage.

**W/A**: to build custom image and to deploy it along with others as usual.

### How to use it?

```bash
$ ./build-n-push.sh

```

[1]: https://www.portainer.io/
[2]: https://github.com/portainer/portainer/issues/2046#issuecomment-405337444
