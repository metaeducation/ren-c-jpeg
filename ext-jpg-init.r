Rebol [
    title: "JPG Codec Extension"
    name: JPG
    type: module
    version: 1.0.0
    license: "Apache 2.0"
]

sys.util/register-codec 'jpeg [%.jpg %jpeg]
    identify-jpeg?/
    decode-jpeg/
    ^void  ; currently no JPG encoder
