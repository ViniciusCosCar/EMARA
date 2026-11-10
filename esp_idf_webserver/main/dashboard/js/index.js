// map and pos. init.
var pos = [40.179479762376175, -8.418097439674227];
var map = L.map('map');
map.setView(pos, 13);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

var waypoints = [pos];
var marker = L.marker(pos).addTo(map);
var polyline = L.polyline([pos]).addTo(map);
var popup = L.popup()

function clearTrajectory() {
        waypoints = [pos];
        polyline.setLatLngs([pos])
}

// "click on map" callback
map.on("click", (e) => {
        // notify the user, update trajectories array, update drawn polyline
        popup
                .setLatLng(e.latlng)
                .setContent(e.latlng.toString())
                .openOn(map);
        waypoints.push(e.latlng);
        polyline.addLatLng(e.latlng);
        console.log(waypoints);
});

// add event listener to buttons
document.querySelector("#clearTrajectoryButton").addEventListener("click", () => {
        clearTrajectory();
        console.log("trajectory cleared");
})
document.querySelector("#sendTrajectoryButton").addEventListener("click", () => {

        // trajectory mode
        const mode = Number(document.querySelector("#trajectoryMode").value);

        // trajectory
        const trajectory = {
                mode: mode,
                waypoints: waypoints.map(point => ({
                        lat: point.lat,
                        lng: point.lng
                }))
        };

        // post request
        fetch("/trajectory", {
                method: "POST",
                headers: {
                        "Content-Type": "application/json"
                },
                body: JSON.stringify(trajectory)
        }).then((res) => {
                if (!res.ok) {
                        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
                }

                console.log("Trajectory sent successfully.");
                clearTrajectory();

        }).catch((err) => {
                console.log(`Failed to send trajectory: ${err.message}.`);
        })
})

