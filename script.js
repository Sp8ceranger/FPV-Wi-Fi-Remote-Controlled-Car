// Adresse IP du serveur
const ip = window.location.hostname;
document.getElementById('ip').textContent = ip;

// Variables pour le joystick
let joystick = document.getElementById('joystick');
let stick = document.getElementById('stick');
let isDragging = false;
let joystickCenterX = joystick.offsetWidth / 2;
let joystickCenterY = joystick.offsetHeight / 2;
let stickRadius = stick.offsetWidth / 2;
let joystickRadius = joystick.offsetWidth / 2 - stickRadius;

// Variables pour les valeurs
let currentX = 0;
let currentY = 0;
let currentSpeed = 100;

// Connexion WebSocket
let socket;

// Initialisation
function init() {
    // Connexion WebSocket
    socket = new WebSocket('ws://' + ip + ':81/');
    socket.onopen = function() {
        console.log('Connecté au WebSocket');
    };
    socket.onclose = function() {
        console.log('Déconnecté du WebSocket');
        setTimeout(init, 1000); // Reconnexion après 1 seconde
    };
    socket.onerror = function(error) {
        console.error('Erreur WebSocket:', error);
    };
    
    // Événements du joystick
    stick.addEventListener('mousedown', startDrag);
    stick.addEventListener('touchstart', startDrag, { passive: false });
    
    // Événements pour le slider
    let speedSlider = document.getElementById('speed');
    speedSlider.addEventListener('input', function() {
        currentSpeed = this.value;
        document.getElementById('speed-value').textContent = currentSpeed + '%';
        sendData();
    });
    
    // Mise à jour initiale
    sendData();
}

// Début du glisser-déposer
function startDrag(e) {
    isDragging = true;
    e.preventDefault();
    
    // Position initiale
    let rect = joystick.getBoundingClientRect();
    let clientX = e.clientX || e.touches[0].clientX;
    let clientY = e.clientY || e.touches[0].clientY;
    let centerX = rect.left + joystickCenterX;
    let centerY = rect.top + joystickCenterY;
    
    // Calcul des coordonnées relatives
    let x = clientX - centerX;
    let y = clientY - centerY;
    
    // Limite au cercle du joystick
    let distance = Math.sqrt(x * x + y * y);
    if (distance > joystickRadius) {
        x = (x / distance) * joystickRadius;
        y = (y / distance) * joystickRadius;
    }
    
    // Mise à jour de la position
    currentX = Math.round((x / joystickRadius) * 100);
    currentY = Math.round((y / joystickRadius) * 100);
    
    // Positionnement du stick
    stick.style.left = (centerX + x - stickRadius) + 'px';
    stick.style.top = (centerY + y - stickRadius) + 'px';
    
    // Affichage des valeurs
    document.getElementById('x-value').textContent = currentX;
    document.getElementById('y-value').textContent = currentY;
    
    // Envoi des données
    sendData();
    
    // Événements de déplacement
    document.addEventListener('mousemove', drag);
    document.addEventListener('touchmove', drag, { passive: false });
    document.addEventListener('mouseup', stopDrag);
    document.addEventListener('touchend', stopDrag);
}

// Glisser-déposer
function drag(e) {
    if (!isDragging) return;
    e.preventDefault();
    
    let rect = joystick.getBoundingClientRect();
    let clientX = e.clientX || e.touches[0].clientX;
    let clientY = e.clientY || e.touches[0].clientY;
    let centerX = rect.left + joystickCenterX;
    let centerY = rect.top + joystickCenterY;
    
    // Calcul des coordonnées relatives
    let x = clientX - centerX;
    let y = clientY - centerY;
    
    // Limite au cercle du joystick
    let distance = Math.sqrt(x * x + y * y);
    if (distance > joystickRadius) {
        x = (x / distance) * joystickRadius;
        y = (y / distance) * joystickRadius;
    }
    
    // Mise à jour de la position
    currentX = Math.round((x / joystickRadius) * 100);
    currentY = Math.round((y / joystickRadius) * 100);
    
    // Positionnement du stick
    stick.style.left = (centerX + x - stickRadius) + 'px';
    stick.style.top = (centerY + y - stickRadius) + 'px';
    
    // Affichage des valeurs
    document.getElementById('x-value').textContent = currentX;
    document.getElementById('y-value').textContent = currentY;
    
    // Envoi des données
    sendData();
}

// Fin du glisser-déposer
function stopDrag() {
    isDragging = false;
    
    // Retour au centre
    currentX = 0;
    currentY = 0;
    
    stick.style.left = '50%';
    stick.style.top = '50%';
    stick.style.transform = 'translate(-50%, -50%)';
    
    document.getElementById('x-value').textContent = '0';
    document.getElementById('y-value').textContent = '0';
    
    // Envoi des données
    sendData();
    
    // Suppression des écouteurs
    document.removeEventListener('mousemove', drag);
    document.removeEventListener('touchmove', drag);
    document.removeEventListener('mouseup', stopDrag);
    document.removeEventListener('touchend', stopDrag);
}

// Envoi des données au serveur
function sendData() {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send('X:' + currentX + ',Y:' + currentY + ',S:' + currentSpeed);
    }
}

// Initialisation au chargement
window.onload = init;
