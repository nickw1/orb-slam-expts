const canvas1 = document.getElementById('canvas1');
const ctx = canvas1.getContext('2d');
const video1 = document.getElementById('video1');
const width = 640, height = 480;
const FPS = 30;
let wasmPassTime, wasmPassLast = 0;
let active = false, nKeyFrames = 0; 
let ptr;
let pollHandle = null;
let poseMatrix = null; 

navigator.mediaDevices.getUserMedia({video: true}).then (stream => {
    video1.srcObject = stream;
    video1.play();
    setTimeout(processVideo, 500);    
    document.getElementById("start").addEventListener("click", e=> {
        document.getElementById("start").setAttribute("disabled", true);
        if(nKeyFrames === 0) {
            status('Press the button to capture the first key frame.');
            document.getElementById("captureKeyFrame").removeAttribute("disabled");
        }
        document.getElementById("stop").removeAttribute("disabled");
        active = true;
        pollHandle = setInterval(pollORB, 30000);
    });
    document.getElementById("captureKeyFrame").addEventListener("click", e=> {
        const ret = Module._captureKeyFrame();    
        console.log(`_captureKeyFrame() returned: ${ret}`);
        if(++nKeyFrames == 1) {
            status('Press the button again to capture the next keyframe.');
            document.getElementById("captureKeyFrame").value = 'Capture key frame 2';
        } else {
            status('Both keyframes captured, tracking...');
            document.getElementById("captureKeyFrame").setAttribute("disabled", true);
        }
    });
    document.getElementById("stop").addEventListener("click", e=> {
        active = false;
        document.getElementById("start").removeAttribute("disabled");
        document.getElementById("stop").setAttribute("disabled", true);
        clearInterval(pollHandle);
        pollHandle = null;
    });
    document.getElementById("cleanup").addEventListener("click", e=> {
        Module._cleanup();
        Module._free(ptr);
        document.getElementById("start").setAttribute("disabled", true);
        document.getElementById("captureKeyFrame").setAttribute("disabled", true);
        document.getElementById("stop").setAttribute("disabled", true);
        document.getElementById("cleanup").setAttribute("disabled", true);
        console.log('memory freed');
    });
});

function processVideo() {
    const begin = Date.now();
    ctx.drawImage(video1, 0, 0, width, height);
    wasmPassTime = Date.now();
    if(active && wasmPassTime - wasmPassLast > 1000) {
        sendCanvas(document.getElementById('canvas1'));
        wasmPassLast = wasmPassTime;
    }
    const delay = 1000/FPS - (Date.now() - begin);
    setTimeout(processVideo, delay);
}

function sendCanvas(canvas) {
    const ctx = canvas.getContext('2d');
    const imgData = ctx.getImageData(0, 0, width, height);
    const uint8ArrData = new Uint8Array(imgData.data);
    passToWasm(uint8ArrData, width, height);
}

function passToWasm(data, width, height) {
    console.log('passToWasm()');
    if(!ptr) {    
        ptr = Module._malloc(data.length * data.BYTES_PER_ELEMENT);
    }
    Module.HEAPU8.set(data, ptr);
    try {
        const needReset = Module._receiveData(ptr, width, height);
        if(needReset) {
            nKeyFrames = 0;
            document.getElementById("captureKeyFrame").removeAttribute("disabled");
            document.getElementById("captureKeyFrame").value = 'Capture key frame 1';
            status('Could not track - resetting, please capture first key frame again.');
        }
    } catch(e) { 
       console.log(e); 
    }
}

function pollORB() {
    // TODO
}

function status(msg) {
    document.getElementById('status').innerHTML = msg;
}

