"use strict";
var cubes = [[
        { x: 0, y: 0 }, { x: 100, y: 0 }, { x: 100, y: 100 }, { x: 0, y: 100 }
    ], [
        { x: 200, y: 200 }, { x: 300, y: 200 }, { x: 300, y: 300 }, { x: 200, y: 300 }
    ]];
function normalize(v1) {
    var l = Math.sqrt(Math.pow(v1.x, 2) + Math.pow(v1.y, 2));
    return { x: v1.x / l, y: v1.y / l };
}
function dotProduct(v1, v2) {
    return v1.x * v2.x + v1.y * v2.y;
}
// Compute the cross product of vectors (p1p2) and (p1p3)
function crossProduct(p1, p2, p3) {
    var dx1 = p2.x - p1.x;
    var dy1 = p2.y - p1.y;
    var dx2 = p3.x - p1.x;
    var dy2 = p3.y - p1.y;
    return dx1 * dy2 - dy1 * dx2;
}
function subtractVectors(v1, v2) {
    return {
        x: v1.x - v2.x,
        y: v1.y - v2.y,
    };
}
function calculateHull(points, uvst) {
    var allPoints = [];
    for (var i = 0; i < 4; ++i) {
        for (var j = 0; j < 4; ++j) {
            allPoints[i * 4 + j] = subtractVectors(points[i], uvst[j]);
        }
    }
    allPoints.sort(function (a, b) { return a.x - b.x || a.y - b.y; });
    // Compute the lower hull
    var lowerHull = [];
    for (var _i = 0, allPoints_1 = allPoints; _i < allPoints_1.length; _i++) {
        var point = allPoints_1[_i];
        while (lowerHull.length >= 2 && crossProduct(lowerHull[lowerHull.length - 2], lowerHull[lowerHull.length - 1], point) <= 0) {
            lowerHull.pop();
        }
        lowerHull.push(point);
    }
    // Compute the upper hull
    var upperHull = [];
    for (var i = allPoints.length - 1; i >= 0; --i) {
        var point = allPoints[i];
        while (upperHull.length >= 2 && crossProduct(upperHull[upperHull.length - 2], upperHull[upperHull.length - 1], point) <= 0) {
            upperHull.pop();
        }
        upperHull.push(point);
    }
    // Combine lower and upper hulls (excluding duplicate points)
    var hull = lowerHull.concat(upperHull.slice(1, -1));
    return hull;
}
// Helper function to calculate if a point is left of a line
function isLeft(p1, p2, p3) {
    return crossProduct(p1, p2, p3);
}
function isPointInsideHull(point, hull) {
    var windingNumber = 0;
    var n = hull.length;
    for (var i = 0; i < n; ++i) {
        var p1 = hull[i];
        var p2 = hull[(i + 1) % n];
        if (p1.y <= point.y) {
            if (p2.y > point.y && isLeft(p1, p2, point) > 0) {
                windingNumber++;
            }
        }
        else {
            if (p2.y <= point.y && isLeft(p1, p2, point) < 0) {
                windingNumber--;
            }
        }
    }
    return windingNumber !== 0;
}
function inChannel(points, uvst, ctx) {
    var intersects = false;
    var hull = calculateHull(points, uvst);
    // Check if the origin lies inside the hull
    var origin = { x: 0, y: 0 };
    if (isPointInsideHull(origin, hull)) {
        intersects = true;
    }
    drawHull(ctx, hull);
    return intersects;
}
// Add an array to keep track of handle positions
var handles = [];
// Function to initialize handle positions based on cube points
function initializeHandles() {
    handles.length = 0; // Clear handles array
    var c = 0;
    for (var _i = 0, cubes_1 = cubes; _i < cubes_1.length; _i++) {
        var cube = cubes_1[_i];
        var i = 0;
        for (var _a = 0, cube_1 = cube; _a < cube_1.length; _a++) {
            var point = cube_1[_a];
            handles.push({ x: point.x, y: point.y, idx: i++, cube: c });
        }
        c++;
    }
}
// Add event listeners to handle mouse events
function addMouseListeners(canvas, ctx) {
    var dragging = false;
    var selectedHandleIndex = -1;
    canvas.addEventListener('mousedown', function (event) {
        var mouseX = event.clientX - canvas.getBoundingClientRect().left;
        var mouseY = event.clientY - canvas.getBoundingClientRect().top;
        var origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
        mouseX -= origin.x;
        mouseY -= origin.y;
        // Check if the mouse is over any handle
        for (var i = 0; i < handles.length; i++) {
            var handle = handles[i];
            var distance = Math.sqrt(Math.pow(mouseX - handle.x, 2) + Math.pow(mouseY - handle.y, 2));
            if (distance <= 25) { // You can adjust this threshold
                selectedHandleIndex = i;
                dragging = true;
                break;
            }
        }
    });
    canvas.addEventListener('mousemove', function (event) {
        if (dragging && selectedHandleIndex !== -1) {
            var origin_1 = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
            var h = handles[selectedHandleIndex];
            handles[selectedHandleIndex].x = event.clientX - canvas.getBoundingClientRect().left - origin_1.x;
            handles[selectedHandleIndex].y = event.clientY - canvas.getBoundingClientRect().top - origin_1.y;
            cubes[h.cube][h.idx].x = h.x;
            cubes[h.cube][h.idx].y = h.y;
            drawCubes(ctx);
        }
    });
    canvas.addEventListener('mouseup', function () {
        dragging = false;
        selectedHandleIndex = -1;
    });
}
function drawHull(ctx, points) {
    var origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "blue";
    for (var j = 0; j < points.length; ++j) {
        var _a = points[j], x = _a.x, y = _a.y;
        var nextIndex = (j + 1) % points.length;
        var _b = points[nextIndex], nextX = _b.x, nextY = _b.y;
        ctx.moveTo(x + origin.x, y + origin.y);
        ctx.lineTo(nextX + origin.x, nextY + origin.y);
    }
    ctx.stroke();
}
;
function drawCubes(ctx) {
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    var intersects1 = inChannel(cubes[0], cubes[1], ctx);
    var origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "black";
    ctx.moveTo(0, origin.y);
    ctx.lineTo(ctx.canvas.width, origin.y);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(origin.x, 0);
    ctx.lineTo(origin.x, ctx.canvas.height);
    ctx.stroke();
    console.log("Cube 1 intersects Cube 2:", intersects1);
    for (var i = 0; i < cubes.length; ++i) {
        ctx.beginPath();
        ctx.lineWidth = 3;
        for (var j = 0; j < cubes[i].length; ++j) {
            var _a = cubes[i][j], x = _a.x, y = _a.y;
            var nextIndex = (j + 1) % cubes[i].length;
            var _b = cubes[i][nextIndex], nextX = _b.x, nextY = _b.y;
            ctx.moveTo(x + origin.x, y + origin.y);
            ctx.lineTo(nextX + origin.x, nextY + origin.y);
        }
        ctx.strokeStyle = intersects1 ? "red" : "green";
        ctx.stroke();
    }
    // Draw handles
    for (var _i = 0, handles_1 = handles; _i < handles_1.length; _i++) {
        var _c = handles_1[_i], x = _c.x, y = _c.y;
        ctx.beginPath();
        ctx.arc(x + origin.x, y + origin.y, 10, 0, Math.PI * 2);
        ctx.fillStyle = "red";
        ctx.fill();
    }
}
// Function to draw the cube with handles
function drawCube(ctx, cube, start) {
    // Draw cube edges
    ctx.beginPath();
    var l = cube.length + start;
    for (var i = start; i < l; ++i) {
        var _a = handles[i], x = _a.x, y = _a.y;
        var nextIndex = ((i + 1) == l) ? start : i + 1;
        var _b = handles[nextIndex], nextX = _b.x, nextY = _b.y;
        ctx.moveTo(x, y);
        ctx.lineTo(nextX, nextY);
    }
    ctx.strokeStyle = "black";
    ctx.stroke();
    // Draw handles
    for (var _i = 0, handles_2 = handles; _i < handles_2.length; _i++) {
        var _c = handles_2[_i], x = _c.x, y = _c.y;
        ctx.beginPath();
        ctx.arc(x, y, 5, 0, Math.PI * 2);
        ctx.fillStyle = "red";
        ctx.fill();
    }
}
// Your existing code here...
// Main function
function main() {
    var canvas = document.getElementById("myCanvas");
    var ctx = canvas.getContext("2d");
    if (!ctx) {
        console.error("Canvas not supported!");
        return;
    }
    // Initialize handle positions
    initializeHandles();
    // Add mouse event listeners
    addMouseListeners(canvas, ctx);
    drawCubes(ctx);
}
main();
