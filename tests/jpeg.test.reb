; jpeg.test.reb

(image? decode 'jpeg read %../fixtures/rebol-logo.jpg)

; The results of decoding lossless encodings should be identical.
(
    bmp-img: decode 'bmp read %../fixtures/rebol-logo.bmp
    gif-img: decode 'gif read %../fixtures/rebol-logo.gif
    png-img: decode 'png read %../fixtures/rebol-logo.png
    all [
        bmp-img == gif-img
        bmp-img == png-img
    ]
)
