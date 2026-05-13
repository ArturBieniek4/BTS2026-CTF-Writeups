The given image file carries a ZIP archive with it.
The ZIP is encrypted but can easily be cracked using `john`.
We get the next image.
We need to flatten each pixel into RGB bytes, arrange them in order and get LSB of every even-indexed byte (hence the hint in the filename 2n). We then put those bits into bytes in MSB-first order and get a base64, which decodes to the flag contents.
