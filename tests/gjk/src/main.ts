interface Vector2 {
	x: number;
	y: number;
}

const cubes: Vector2[][] = [[
	{ x: 0, y: 0 }, { x: 100, y: 0 }, { x: 100, y: 100 }, { x: 0, y: 100 }
], [
	{ x: 200, y: 200 }, { x: 300, y: 200 }, { x: 300, y: 300 }, { x: 200, y: 300 }
]];

function normalize(v1: Vector2): Vector2 {
	let l = Math.sqrt(v1.x ** 2 + v1.y ** 2);
	return { x: v1.x / l, y: v1.y / l };
}

function dotProduct(v1: Vector2, v2: Vector2): number {
	return v1.x * v2.x + v1.y * v2.y;
}


// Compute the cross product of vectors (p1p2) and (p1p3)
function crossProduct(p1: Vector2, p2: Vector2, p3: Vector2): number {
    const dx1 = p2.x - p1.x;
    const dy1 = p2.y - p1.y;
    const dx2 = p3.x - p1.x;
    const dy2 = p3.y - p1.y;
    return dx1 * dy2 - dy1 * dx2;
}



function subtractVectors(v1: Vector2, v2: Vector2): Vector2 {
	return {
		x: v1.x - v2.x,
		y: v1.y - v2.y,
	};
}

function calculateHull(points: Vector2[], uvst: Vector2[]): Vector2[] {
	
	const allPoints: Vector2[] = [];

	for (let i = 0; i < 4; ++i) {
		for (let j = 0; j < 4; ++j) {
			allPoints[i * 4 + j] = subtractVectors(points[i], uvst[j]);
		}
	}
	allPoints.sort((a, b) => a.x - b.x || a.y - b.y);

	// Compute the lower hull
	const lowerHull = [];
	for (const point of allPoints) {
		while (lowerHull.length >= 2 && crossProduct(lowerHull[lowerHull.length - 2], lowerHull[lowerHull.length - 1], point) <= 0) {
			lowerHull.pop();
		}
		lowerHull.push(point);
	}

	// Compute the upper hull
	const upperHull = [];
	for (let i = allPoints.length - 1; i >= 0; --i) {
		const point = allPoints[i];
		while (upperHull.length >= 2 && crossProduct(upperHull[upperHull.length - 2], upperHull[upperHull.length - 1], point) <= 0) {
			upperHull.pop();
		}
		upperHull.push(point);
	}

	// Combine lower and upper hulls (excluding duplicate points)
	const hull = lowerHull.concat(upperHull.slice(1, -1));

	return hull;
}

// Helper function to calculate if a point is left of a line
function isLeft(p1: Vector2, p2: Vector2, p3: Vector2): number {
    return crossProduct(p1, p2, p3);
}

function isPointInsideHull(point: Vector2, hull: Vector2[]): boolean {
    let windingNumber = 0;
    const n = hull.length;
    
    for (let i = 0; i < n; ++i) {
        const p1 = hull[i];
        const p2 = hull[(i + 1) % n];
        
        if (p1.y <= point.y) {
            if (p2.y > point.y && isLeft(p1, p2, point) > 0) {
                windingNumber++;
            }
        } else {
            if (p2.y <= point.y && isLeft(p1, p2, point) < 0) {
                windingNumber--;
            }
        }
    }
    
    return windingNumber !== 0;
}

function inChannel(points: Vector2[], uvst: Vector2[], ctx: CanvasRenderingContext2D): boolean {
	let intersects = false;
	const hull = calculateHull(points, uvst);

	// Check if the origin lies inside the hull
	const origin = { x: 0, y: 0 };
	if (isPointInsideHull(origin, hull)) {
		intersects = true;
	}

	drawHull(ctx, hull); return intersects;
}


// Add an array to keep track of handle positions
const handles: { x: number, y: number, idx: number, cube: number }[] = [];

// Function to initialize handle positions based on cube points
function initializeHandles() {
	handles.length = 0; // Clear handles array
	let c: number = 0;
	for (const cube of cubes) {
		let i: number = 0;
		for (const point of cube) {
			handles.push({ x: point.x, y: point.y, idx: i++, cube: c });
		}
		c++;
	}
}

// Add event listeners to handle mouse events
function addMouseListeners(canvas: HTMLCanvasElement, ctx: CanvasRenderingContext2D) {
	let dragging = false;
	let selectedHandleIndex = -1;

	canvas.addEventListener('mousedown', (event) => {
		let mouseX = event.clientX - canvas.getBoundingClientRect().left;
		let mouseY = event.clientY - canvas.getBoundingClientRect().top;
		let origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
		mouseX -= origin.x;
		mouseY -= origin.y;

		// Check if the mouse is over any handle
		for (let i = 0; i < handles.length; i++) {
			const handle = handles[i];
			const distance = Math.sqrt(Math.pow(mouseX - handle.x, 2) + Math.pow(mouseY - handle.y, 2));
			if (distance <= 25) { // You can adjust this threshold
				selectedHandleIndex = i;
				dragging = true;
				break;
			}
		}
	});

	canvas.addEventListener('mousemove', (event) => {
		if (dragging && selectedHandleIndex !== -1) {
			let origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
			let h = handles[selectedHandleIndex];
			handles[selectedHandleIndex].x = event.clientX - canvas.getBoundingClientRect().left - origin.x;
			handles[selectedHandleIndex].y = event.clientY - canvas.getBoundingClientRect().top - origin.y;
			cubes[h.cube][h.idx].x = h.x;
			cubes[h.cube][h.idx].y = h.y;
			drawCubes(ctx);
		}
	});

	canvas.addEventListener('mouseup', () => {
		dragging = false;
		selectedHandleIndex = -1;
	});
}

function drawHull(ctx: CanvasRenderingContext2D, points: Vector2[]) {
	let origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };
	ctx.beginPath();
	ctx.lineWidth = 1;
	ctx.strokeStyle = "blue";
	for (let j = 0; j < points.length; ++j) {
		const { x, y } = points[j];
		const nextIndex = (j + 1) % points.length;
		const { x: nextX, y: nextY } = points[nextIndex];
		ctx.moveTo(x + origin.x, y + origin.y);
		ctx.lineTo(nextX + origin.x, nextY + origin.y);
	}
	ctx.stroke();
};

function drawCubes(ctx: CanvasRenderingContext2D) {
	ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
	const intersects1 = inChannel(cubes[0], cubes[1], ctx);

	let origin = { x: ctx.canvas.width / 2, y: ctx.canvas.height / 2 };

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

	for (let i = 0; i < cubes.length; ++i) {
		ctx.beginPath();
		ctx.lineWidth = 3;
		for (let j = 0; j < cubes[i].length; ++j) {
			const { x, y } = cubes[i][j];
			const nextIndex = (j + 1) % cubes[i].length;
			const { x: nextX, y: nextY } = cubes[i][nextIndex];
			ctx.moveTo(x + origin.x, y + origin.y);
			ctx.lineTo(nextX + origin.x, nextY + origin.y);
		}
		ctx.strokeStyle = intersects1 ? "red" : "green";
		ctx.stroke();
	}

	// Draw handles
	for (const { x, y } of handles) {
		ctx.beginPath();
		ctx.arc(x + origin.x, y + origin.y, 10, 0, Math.PI * 2);
		ctx.fillStyle = "red";
		ctx.fill();
	}

}

// Function to draw the cube with handles
function drawCube(ctx: CanvasRenderingContext2D, cube: Vector2[], start: number) {
	// Draw cube edges
	ctx.beginPath();
	let l = cube.length + start;
	for (let i = start; i < l; ++i) {
		const { x, y } = handles[i];
		const nextIndex = ((i + 1) == l) ? start : i + 1;
		const { x: nextX, y: nextY } = handles[nextIndex];
		ctx.moveTo(x, y);
		ctx.lineTo(nextX, nextY);
	}
	ctx.strokeStyle = "black";
	ctx.stroke();

	// Draw handles
	for (const { x, y } of handles) {
		ctx.beginPath();
		ctx.arc(x, y, 5, 0, Math.PI * 2);
		ctx.fillStyle = "red";
		ctx.fill();
	}
}

// Your existing code here...

// Main function
function main() {
	const canvas = document.getElementById("myCanvas") as HTMLCanvasElement;
	const ctx = canvas.getContext("2d");
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
