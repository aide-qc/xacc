/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#include "py_graph.hpp"

#include "xacc.hpp"
#include "pybind11/stl.h"
#include "xacc_service.hpp"
#include "py_heterogeneous_map.hpp"

void bind_graph(py::module &m) {
  // Expose xacc::Graph
  py::class_<xacc::Graph, std::shared_ptr<xacc::Graph>, PyGraph> graph(
      m, "Graph", "Graph wraps the XACC C++ Graph class");
  graph.def(py::init<>())
      .def("name", &xacc::Graph::name, "")
      .def("description", &xacc::Graph::description, "")
      .def("clone", &xacc::Graph::clone, "")
      .def("gen_random_graph", &xacc::Graph::gen_random_graph, "")
      .def("addEdge",
           (void (xacc::Graph::*)(const int, const int, const double)) &
               xacc::Graph::addEdge,
           "")
      .def("addEdge",
           (void (xacc::Graph::*)(const int, const int)) & xacc::Graph::addEdge,
           "")
      .def("removeEdge", &xacc::Graph::removeEdge, "")
      .def(
          "addVertex",
          [](xacc::Graph &g, const PyHeterogeneousMap &vertex) {
            HeterogeneousMap m;
            for (auto &item : vertex) {
              PyHeterogeneousMap2HeterogeneousMap vis(m, item.first);
              mpark::visit(vis, item.second);
            }
            g.addVertex(m);
          },
          "")
      .def(
          "setVertexProperties",
          [](xacc::Graph &g, const int index,
             const PyHeterogeneousMap &vertex) {
            HeterogeneousMap m;
            for (auto &item : vertex) {
              PyHeterogeneousMap2HeterogeneousMap vis(m, item.first);
              mpark::visit(vis, item.second);
            }
            g.setVertexProperties(index, m);
          },
          "")
      .def("getVertexProperties", &xacc::Graph::getVertexProperties, "")
      .def("setEdgeWeight", &xacc::Graph::setEdgeWeight, "")
      .def("getEdgeWeight", &xacc::Graph::getEdgeWeight, "")
      .def("edgeExists", &xacc::Graph::edgeExists, "")
      .def("degree", &xacc::Graph::degree, "")
      .def("diameter", &xacc::Graph::diameter, "")
      .def("size", &xacc::Graph::size, "")
      .def("order", &xacc::Graph::order, "")
      .def("depth", &xacc::Graph::depth, "")
      .def("getNeighborList", &xacc::Graph::getNeighborList, "")
      .def("computeShortestPath", &xacc::Graph::computeShortestPath, "")
      .def(
          "__str__",
          [](xacc::Graph &g) {
            std::stringstream ss;
            g.write(ss);
            return ss.str();
          },
          "");

  m.def("getGraph",
        [](const std::string &name) -> std::shared_ptr<xacc::Graph> {
          auto graph = xacc::getService<xacc::Graph>(name);
          return graph;
        });
  // Convert graph from networkx graph type to XACC
  m.def("convertGraph", 
        [] (py::object object) -> std::shared_ptr<xacc::Graph> {
          // Throw err if missing attrs
          if (!py::hasattr(object, "nodes")) {
              xacc::error("\n 'nodes' attribute needed to convert networkx to xacc graph\n");
          }
          if (!py::hasattr(object, "edges")) {
              xacc::error("\n 'edges' attribute needed to convert networkx to xacc graph\n");
          }
          auto nodes = object.attr("nodes");
          auto edges = object.attr("edges").cast<std::tuple<int,int>();
          // Instantiate xacc graph
          auto const xacc_graph = xacc::getService<xacc::Graph>("boost-digraph");
          // Add nodes
          for (auto node : nodes.cast<py::array>()) {
              HeterogeneousMap m;
              m.insert("name", node.cast<py::str>());
              xacc_graph->addVertex(m);
          }
          // Add edges
          int i = 0;
          std::cout << "edges: " << edges << std::endl;
          std::cout << "edge: " << edges(0) << std::endl;
          for (auto edge : edges){
            std::cout << "edge: " << edge << std::endl;
            std::cout << "edge 0: " << edge(0) << std::endl;
            std::cout << "edge 1: " << edge(1) << std::endl;
            // if (weight doesn't exist){
                // set weight = 1.0
                // append to xacc_graph
            // } else {
                // xacc_graph->addEdge(edge(0), edge(1), edge(2));
            // }   
            xacc_graph->addEdge(0, i, 1.);
            i+=1;
          }
          return xacc_graph;
    });
}